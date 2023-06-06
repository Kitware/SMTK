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
#include "smtk/attribute/Item.h"
#include "smtk/attribute/Resource.h"
#include "smtk/io/Logger.h"
#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"
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
}

bool ConfigureOperation::reconfigureTask()
{
  // Interim location for logic to copy values
  // std::cout << __FILE__ << ":" << __LINE__ << " " << std::endl;

  // Input task - must be FillOutAttributes
  auto* fromTask = dynamic_cast<FillOutAttributes*>(this->from());
  if (fromTask == nullptr)
  {
    smtkErrorMacro(
      defaultLogger, "ConfigureOperation \"from\" task is not type FillOutAttributes.");
    return false;
  }

  // Traverse parameter sets
  for (auto& paramSet : m_parameterSets)
  {
    // Find matching attribute set (gotta use visitor pattern, of course)
    bool foundMatch = false;
    fromTask->visitAttributeSets(
      [this, &foundMatch, &paramSet](
        const smtk::task::FillOutAttributes::AttributeSet& attSet) -> smtk::common::Visit {
        if (attSet.m_role == paramSet.m_fromRole)
        {
          foundMatch = true;
          this->updateOperation(attSet, paramSet);
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

    // std::cout << __FILE__ << ":" << __LINE__ << " " << foundMatch << std::endl;
  }

  return false;
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

void ConfigureOperation::updateOperation(
  const smtk::task::FillOutAttributes::AttributeSet& attSet,
  const ParameterSet& paramSet)
{
  // std::cout << __FILE__ << ":" << __LINE__ << " " << std::endl;

  // Get the operation
  auto* operationTask = dynamic_cast<smtk::task::SubmitOperation*>(this->to());
  if (operationTask == nullptr)
  {
    smtkWarningMacro(defaultLogger, "ConfigureOperation \"to\" task is not SubmitOperation.");
    return;
  }
  auto* operation = operationTask->operation();
  if (operation == nullptr)
  {
    smtkWarningMacro(
      defaultLogger,
      "SubmitOperation task \"" << operationTask->title()
                                << "\" not configured to an Operation instance.");
    return;
  }

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
    smtkWarningMacro(
      defaultLogger, "attribute resource with role \"" << paramSet.m_fromRole << "\"not found.");
    return;
  }

  // Process paramSet
  for (const auto& it : paramSet.m_pathMap)
  {
    const std::string attQuery = it.first;
    const std::string paramPath = it.second;

    // Split the qttQuery into attribute[] and itemPath (is there an easier way?)
    std::size_t n = attQuery.find(']');
    if (n == std::string::npos)
    {
      smtkWarningMacro(
        defaultLogger,
        "ConfigureOperation unexpected from spec \""
          << "\"; expected attribute[type='something'].");
      continue;
    }

    // std::cout << __FILE__ << ":" << __LINE__ << " " << attSet.m_definitions.size() << std::endl;
    // std::cout << __FILE__ << ":" << __LINE__ << " " << attSet.m_instances.size() << std::endl;

    std::string attributePart = attQuery.substr(0, n + 1);
    // std::cout << __FILE__ << ":" << __LINE__ << " " << attributePart << std::endl;
    std::string itemPart = attQuery.substr(n + 2);
    // std::cout << __FILE__ << ":" << __LINE__ << " " << itemPart << std::endl;

    // Find the attribute & item in the FillOutAttribute tasks and assign to parameter
    auto queryOp = attResource->queryOperation(attributePart);
    for (const auto& attName : attSet.m_instances)
    {
      auto fromAtt = attResource->findAttribute(attName);
      // std::cout << __FILE__ << ":" << __LINE__ << " " << attName << (att != nullptr) << std::endl;
      if (fromAtt != nullptr && queryOp(*fromAtt))
      {
        // std::cout << __FILE__ << ":" << __LINE__ << " " << att->name() << ", " << att->type() << std::endl;
        // Get the "from" item
        auto fromItem = fromAtt->itemAtPath(itemPart);
        if (fromItem == nullptr)
        {
          smtkWarningMacro(
            defaultLogger,
            "ConfigureOperation did not find attribute \" " << attName << "\" itemAtPath \""
                                                            << itemPart << "\"");
          continue;
        }

        // Get the parameter
        auto paramItem = operation->parameters()->itemAtPath(paramPath);
        if (paramItem == nullptr)
        {
          smtkWarningMacro(
            defaultLogger,
            "ConfigureOperation did not find operation parameter at path \" " << paramPath << "\"");
          continue;
        }

        // Use assign() method
        smtk::attribute::CopyAssignmentOptions options;
        bool assignOK = paramItem->assign(fromItem, options, defaultLogger);
        if (!assignOK)
        {
          smtkWarningMacro(
            defaultLogger,
            "ConfigureOperation failed to assign parameter at path \" " << paramPath << "\"");
        }

        break;
      }
    } // for (attName)
  }   // for (it)

  // Update operation task state
  operationTask->internalStateChanged(operationTask->computeInternalState());
  // std::cout << __FILE__ << ":" << __LINE__ << " " << std::endl;
}

} // namespace adaptor
} // namespace task
} // namespace smtk
