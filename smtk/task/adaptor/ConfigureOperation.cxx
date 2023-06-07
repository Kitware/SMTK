//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/task/adaptor/ConfigureOperation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/operators/Signal.h"
#include "smtk/common/Managers.h"
#include "smtk/common/TypeName.h"
#include "smtk/io/Logger.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/resource/Manager.h"
#include "smtk/task/Active.h"
#include "smtk/task/Manager.h"
#include "smtk/task/SubmitOperation.h"

#include <utility> // std::move

namespace
{
auto& defaultLogger = smtk::io::Logger::instance();
}

namespace smtk
{
namespace task
{
namespace adaptor
{

ConfigureOperation::ConfigureOperation() = default;
ConfigureOperation::ConfigureOperation(const Configuration& config)
{
  this->configureSelf(config);
}

ConfigureOperation::ConfigureOperation(const Configuration& config, Task* from, Task* to)
  : Superclass(config, from, to)
{
  this->configureSelf(config);

  // Check state of "from" task
  if (from == nullptr)
  {
    return;
  }

  if (from->state() == smtk::task::State::Completable)
  {
    this->buildInternalData(config);
    this->setupAttributeObserver(config);
    m_applyChanges = true;
    this->updateOperation();
  }

  // Add observer for "from" task state changes
  m_taskObserver = from->observers().insert([this, config](Task&, State prev, State next) {
    bool isCompletable = (next == State::Completable);
    if (isCompletable && m_attributeSet.empty())
    {
      this->buildInternalData(config);
      this->setupAttributeObserver(config);
      m_applyChanges = true;
      this->updateOperation();
    }
    else
    {
      m_applyChanges = false;
    }
  });
}

bool ConfigureOperation::reconfigureTask()
{
  return false; // Required override
}

void ConfigureOperation::configureSelf(const Configuration& config)
{
  auto configIter = config.find("configure");
  if (configIter == config.end())
  {
    smtkWarningMacro(defaultLogger, "ConfigureOperation adaptor missing \"configure\" element.");
    return;
  }
  else if (!configIter->is_array())
  {
    smtkWarningMacro(defaultLogger, "ConfigureOperation \"configure\" element is not an array.");
    return;
  }

  for (auto it = configIter->begin(); it != configIter->end(); ++it)
  {
    if (!it->is_object())
    {
      smtkWarningMacro(
        defaultLogger, "ConfigureOperation \"configure\" has element that is not an object.");
      continue;
    }

    ParameterSet paramSet;

    // Traverse all items in the object
    for (auto& el : it->items())
    {
      const std::string& key = el.key();
      const std::string value = el.value().get<std::string>();
      if (key == "from-role")
      {
        paramSet.m_fromRole = value;
      }
      else
      {
        paramSet.m_pathMap[key] = value;
      }
    } // for (el)

    if (paramSet.m_fromRole.empty())
    {
      smtkWarningMacro(
        defaultLogger, "ConfigureOperation attribute-set missing \"from-role\" item.");
    }
    else
    {
      m_parameterSets.push_back(std::move(paramSet));
    }
  } // for (configIter)
}

bool ConfigureOperation::buildInternalData(const Configuration& config)
{
  defaultLogger.clearErrors();
  m_attributeSet.clear();
  m_itemTable.clear();

  // "From" task - must be FillOutAttributes
  auto* fromTask = dynamic_cast<FillOutAttributes*>(this->from());
  if (fromTask == nullptr)
  {
    smtkErrorMacro(
      defaultLogger, "ConfigureOperation \"from\" task is not type FillOutAttributes.");
    return false;
  }

  // "To" task - must be SubmitOperation
  auto* operationTask = dynamic_cast<smtk::task::SubmitOperation*>(this->to());
  if (operationTask == nullptr)
  {
    smtkErrorMacro(defaultLogger, "ConfigureOperation \"to\" task is not SubmitOperation.");
    return false;
  }

  // Get the operation
  auto* operation = operationTask->operation();
  if (operation == nullptr)
  {
    smtkErrorMacro(
      defaultLogger,
      "SubmitOperation task \"" << operationTask->title()
                                << "\" not configured to an Operation instance.");
    return false;
  }

  // Traverse parameter sets
  for (const auto& paramSet : m_parameterSets)
  {
    // Find matching attribute set (gotta use visitor pattern, of course)
    bool foundMatch = false;
    fromTask->visitAttributeSets(
      [this, &foundMatch, &paramSet, operation](
        const smtk::task::FillOutAttributes::AttributeSet& attSet) -> smtk::common::Visit {
        if (attSet.m_role == paramSet.m_fromRole)
        {
          foundMatch = true;
          this->updateInternalData(attSet, paramSet, operation);
          return smtk::common::Visit::Halt;
        }
        return smtk::common::Visit::Continue;
      });

    if (!foundMatch)
    {
      smtkErrorMacro(
        defaultLogger,
        "ConfigureOperation failed to find matching role \"" << paramSet.m_fromRole << "\"");
    }
  }

  bool ok = !defaultLogger.hasErrors();
  // Note: Now that m_itemTable is built (if there were no errors), we could clear m_paramterSets
  return ok;
}

bool ConfigureOperation::updateInternalData(
  const smtk::task::FillOutAttributes::AttributeSet& attSet,
  const ParameterSet& paramSet,
  smtk::operation::Operation* operation)
{
  // Find  the attribute resource (there might be an easier way to do this?)
  smtk::attribute::ResourcePtr attResource;
  auto resManager = operation->resourceManager();
  std::string role = paramSet.m_fromRole;
  resManager->visit(
    [&resManager, &role, &attResource](
      smtk::resource::Resource& resource) -> smtk::common::Processing {
      if (resource.isOfType<smtk::attribute::Resource>())
      {
        if (resource.properties().get<std::string>()["project_role"] == role)
        {
          attResource = resManager->get<smtk::attribute::Resource>(resource.id());
          return smtk::common::Processing::STOP;
        }
      }
      return smtk::common::Processing::CONTINUE;
    });

  if (attResource == nullptr)
  {
    smtkErrorMacro(
      defaultLogger, "attribute resource with role \"" << paramSet.m_fromRole << "\"not found.");
    return false;
  }

  // Process paramSet to get source attribute
  for (const auto& it : paramSet.m_pathMap)
  {
    const std::string attQuery = it.first;
    const std::string paramPath = it.second;

    // Split the qttQuery into attribute[] and itemPath (is there an easier way?)
    std::size_t n = attQuery.find(']');
    if (n == std::string::npos)
    {
      smtkErrorMacro(
        defaultLogger,
        "ConfigureOperation unexpected from spec \""
          << "\"; expected attribute[type='something'].");
      continue;
    }
    std::string attributePart = attQuery.substr(0, n + 1);
    std::string itemPart = attQuery.substr(n + 2);

    // Find the attribute & item in the FillOutAttribute tasks and assign to parameter
    auto queryOp = attResource->queryOperation(attributePart);
    for (const auto& attName : attSet.m_instances)
    {
      auto fromAtt = attResource->findAttribute(attName);
      if (fromAtt != nullptr && queryOp(*fromAtt))
      {
        m_attributeSet.insert(fromAtt->id());
        m_itemTable.emplace_back(fromAtt, itemPart, paramPath);
        break;
      }
    } // for (attName)
  }   // for (it)

  bool ok = !defaultLogger.hasErrors();
  return ok;
}

bool ConfigureOperation::setupAttributeObserver(const Configuration& config)
{
  // Get operation manager from managers
  auto managers = this->from()->manager()->managers();
  auto opManager = managers->get<smtk::operation::Manager::Ptr>();
  m_attributeObserver = opManager->observers().insert(
    [this](
      const smtk::operation::Operation& op,
      smtk::operation::EventType eventType,
      smtk::operation::Operation::Result result) -> int {
      // Most of the time we can ignore the op
      if (!this->m_applyChanges)
      {
        return 0;
      }

      if (eventType != smtk::operation::EventType::DID_OPERATE)
      {
        return 0;
      }

      if (op.typeName() != smtk::common::typeName<smtk::attribute::Signal>())
      {
        return 0;
      }

      // Get the "from" task and check its state
      smtk::task::Task* fromTask = this->from();
      if ((fromTask == nullptr) || fromTask->state() != smtk::task::State::Completable)
      {
        return 0;
      }

      //  See if the "from" task is active
      const auto& active = fromTask->manager()->active();
      if (active.task() != fromTask)
      {
        return 0;
      }

      // Check the result for any attributes in our set
      auto compItem = result->findComponent("modified");
      for (std::size_t i = 0; i < compItem->numberOfValues(); ++i)
      {
        auto compId = compItem->value(i)->id();
        if (m_attributeSet.find(compId) != m_attributeSet.end())
        {
          this->updateOperation();
          break;
        }
      }

      return 1;
    });

  return true;
}

bool ConfigureOperation::updateOperation() const
{
  auto* operationTask = dynamic_cast<smtk::task::SubmitOperation*>(this->to());
  auto* operation = operationTask->operation();

  // Traverse m_itemTable
  for (const auto& t : m_itemTable)
  {
    const smtk::attribute::AttributePtr fromAtt = std::get<0>(t).lock();
    if (fromAtt == nullptr)
    {
      smtkWarningMacro(defaultLogger, "fromAtt is null");
      continue;
    }

    std::string fromItemPath = std::get<1>(t);
    std::string paramItemPath = std::get<2>(t);

    auto fromItem = fromAtt->itemAtPath(fromItemPath);
    if (fromItem == nullptr)
    {
      smtkWarningMacro(
        defaultLogger,
        "ConfigureOperation did not find attribute \" " << fromAtt->name() << "\" itemAtPath \""
                                                        << fromItemPath << "\"");
      continue;
    }

    // Get the parameter
    auto paramItem = operation->parameters()->itemAtPath(paramItemPath);
    if (paramItem == nullptr)
    {
      smtkWarningMacro(
        defaultLogger,
        "ConfigureOperation did not find operation parameter at path \" 0" << paramItemPath
                                                                           << "\"");
      continue;
    }

    // Use assign() method
    smtk::attribute::CopyAssignmentOptions options;
    bool assignOK = paramItem->assign(fromItem, options, defaultLogger);
    if (!assignOK)
    {
      smtkWarningMacro(
        defaultLogger,
        "ConfigureOperation failed to assign parameter at path \" " << paramItemPath << "\"");
    }
  }

  // Update operation task state
  operationTask->internalStateChanged(operationTask->computeInternalState());
  return true;
}

} // namespace adaptor
} // namespace task
} // namespace smtk
