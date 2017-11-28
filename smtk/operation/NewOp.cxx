//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/NewOp.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/SpecificationOps.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/io/SaveJSON.h"

#include "smtk/operation/NewOp_xml.h"

#include "nlohmann/json.hpp"

#include <memory>
#include <sstream>

namespace smtk
{
namespace operation
{

NewOp::NewOp()
  : m_debugLevel(0)
  , m_specification(nullptr)
  , m_parameters(nullptr)
  , m_resultDefinition(nullptr)
  , m_manager(nullptr)
{
}

NewOp::~NewOp()
{
  // If the specification exists...
  if (m_specification != nullptr)
  {
    // ...and if the parameters have been generated, remove the parameters from
    // the specification.
    if (m_parameters != nullptr)
    {
      m_specification->removeAttribute(m_parameters);
    }

    // Similarly, remove all results from the specification that were generated
    // by this operator.
    for (auto& result : m_results)
    {
      m_specification->removeAttribute(result);
    }
  }
}

std::string NewOp::uniqueName() const
{
  if (m_manager)
  {
    // If the operator's manager is set, then the operator is registered to a
    // manager. The operator metadata has a unique name for this operator type,
    // so we return this name.
    auto metadata = m_manager->metadata().get<IndexTag>().find(this->index());
    if (metadata != m_manager->metadata().get<IndexTag>().end())
    {
      return metadata->uniqueName();
    }
  }

  // Either this operator is not registered to a manager or it does not have a
  // unique name registered to it. Simply return the class name.
  return this->classname();
}

NewOp::Specification NewOp::specification()
{
  // Lazily create the specification.
  if (m_specification == nullptr)
  {
    if (m_manager)
    {
      auto metadata = m_manager->metadata().get<IndexTag>().find(this->index());

      // The only way for an operator's manager to be set is if a manager
      // created it. The only way for a manager to create an operator is if it
      // has a metadata instance for its type. Let's check anyway.
      assert(metadata != m_manager->metadata().get<IndexTag>().end());

      m_specification = metadata->specification();
    }
    else
    {
      m_specification = createSpecification();
    }
  }
  return m_specification;
}

bool NewOp::ableToOperate()
{
  return this->parameters()->isValid();
}

NewOp::Result NewOp::operate()
{
  // Gather all requested resources and their permission levels.
  auto resourcesWithPermissions = extractResourcesAndPermissions(this->specification());

  // Lock the resources.
  for (auto& resourceWithPermissions : resourcesWithPermissions)
  {
    auto& resource = resourceWithPermissions.first;
    auto& permission = resourceWithPermissions.second;
    resource->lock({}).lock(permission);
  }

  // Remember where the log was so we only serialize messages for this operation:
  std::size_t logStart = this->log().numberOfRecords();

  Result result;

  // If an operation manager is associated with the operation, call its pre- and
  // post-operation observers. Note that all observers will be called even if
  // one requests the operation be canceled. This is useful since all
  // DID_OPERATE observers are called whether the operation was canceled or not
  // -- and observers of both will expect them to be called in pairs.
  bool observePostOperation = m_manager != nullptr;

  // First, we check that the operator is able to operate.
  if (!this->ableToOperate())
  {
    result = this->createResult(Outcome::UNABLE_TO_OPERATE);
    // If the operator cannot operate, there is no need to call any observers.
    observePostOperation = false;
  }
  // Then, we check if any observers wish to cancel this operation.
  else if (m_manager &&
    m_manager->observers()(shared_from_this(), EventType::WILL_OPERATE, nullptr))
  {
    result = this->createResult(Outcome::CANCELED);
  }
  else
  {
    // Finally, execute the operation.

    // Set the debug level if specified as a convenience for subclasses:
    smtk::attribute::IntItem::Ptr debugItem = this->parameters()->findInt("debug level");
    this->m_debugLevel = ((debugItem && debugItem->isEnabled()) ? debugItem->value() : 0);

    // Perform the derived operation.
    result = this->operateInternal();

    // Post-process the result if the operation was successful.
    int outcome = result->findInt("outcome")->value();
    if (outcome == static_cast<int>(Outcome::SUCCEEDED))
    {
      this->postProcessResult(result);
    }
  }

  // Add a summary of the operation to the result.
  this->generateSummary(result);

  // Now grab all log messages and serialize them into the result attribute.
  {
    std::size_t logEnd = this->log().numberOfRecords();
    if (logEnd > logStart)
    {
      // Serialize relevant log records to a json-formatted string.
      nlohmann::json j = std::vector<smtk::io::Logger::Record>(
        smtk::io::Logger::instance().records().begin() + logStart,
        smtk::io::Logger::instance().records().end());
      result->findString("log")->appendValue(j.dump());
    }
  }

  // Execute post-operation observation
  if (observePostOperation)
  {
    m_manager->observers()(shared_from_this(), EventType::DID_OPERATE, result);
  }

  // Unlock the resources.
  for (auto& resourceWithPermissions : resourcesWithPermissions)
  {
    auto& resource = resourceWithPermissions.first;
    auto& permission = resourceWithPermissions.second;
    resource->lock({}).unlock(permission);
  }

  return result;
}

smtk::io::Logger& NewOp::log() const
{
  return smtk::io::Logger::instance();
}

NewOp::Parameters NewOp::parameters()
{
  if (!m_parameters)
  {
    // Access the operator's specification via its accessor method to ensure that
    // it is created.
    Specification specification = this->specification();

    // Access the base operator definition.
    Definition operatorBase = specification->findDefinition("operator");

    // Find all definitions that derive from the operator definition.
    std::vector<Definition> operatorDefinitions;
    specification->findAllDerivedDefinitions(operatorBase, true, operatorDefinitions);

    smtk::attribute::DefinitionPtr operatorDefinition;
    // If there is only one derived definition, then it is the one we want.
    if (operatorDefinitions.size() == 1)
    {
      operatorDefinition = operatorDefinitions[0];
    }
    else if (!operatorDefinitions.empty())
    {
      // If there are more than one derived definitions, then we pick the one
      // with the same name as our class.
      {
        std::stringstream s;
        s << "Multiple definitions for operator \"" << this->classname()
          << "\" derive from \"operator\". Looking for one with the name \"" << this->classname()
          << "\".";
        smtkWarningMacro(this->log(), s.str());
      }

      for (auto& def : operatorDefinitions)
      {
        if (def->type() == this->classname() ||
          (m_manager != nullptr && def->type() == this->uniqueName()))
        {
          operatorDefinition = def;
          break;
        }
      }
    }

    // Now that we have our operator definition, we create our parameters
    // attribute.
    if (operatorDefinition)
    {
      m_parameters = specification->createAttribute(operatorDefinition);
    }
    else
    {
      std::stringstream s;
      s << "Could not identify parameters attribute definition for operator \"" << this->classname()
        << "\".";
      smtkErrorMacro(this->log(), s.str());
    }
  }
  return m_parameters;
}

NewOp::Result NewOp::createResult(Outcome outcome)
{
  // Our result definition is located once per instance of the operator, and is
  // subsequently retrieved from cache to avoid superfluous lookups.
  if (!m_resultDefinition)
  {
    // Access the operator's specification via its accessor method to ensure that
    // it is created.
    Specification specification = this->specification();

    // Access the base result definition.
    Definition resultBase = specification->findDefinition("result");

    // Find all definitions that derive from the result definition.
    std::vector<Definition> resultDefinitions;
    specification->findAllDerivedDefinitions(resultBase, true, resultDefinitions);

    // If there is only one derived definition, then it is the one we want.
    if (resultDefinitions.size() == 1)
    {
      m_resultDefinition = resultDefinitions[0];
    }
    else if (!resultDefinitions.empty())
    {
      // If there are more than one derived definitions, then we pick the one
      // whose type name is keyed for our operator.
      {
        std::stringstream s;
        s << "Multiple definitions for operator \"" << this->classname()
          << "\" derive from \"result\". Looking for one with the name result(\""
          << this->classname() << ")\"";
        if (m_manager != nullptr && this->classname() != this->uniqueName())
        {
          s << " or result(\"" << this->uniqueName() << ")\".";
        }
        smtkWarningMacro(this->log(), s.str());
      }
      std::string resultClassName;
      {
        std::stringstream s;
        s << "result(" << this->classname() << ")";
        resultClassName = s.str();
      }
      std::string resultUniqueName;
      if (m_manager != nullptr && this->classname() != this->uniqueName())
      {
        std::stringstream s;
        s << "result(" << this->uniqueName() << ")";
        resultUniqueName = s.str();
      }

      for (auto& def : resultDefinitions)
      {
        if (def->type() == resultClassName || def->type() == resultUniqueName)
        {
          m_resultDefinition = def;
          break;
        }
      }
    }
  }

  // Now that we have our result definition, we create our result attribute.
  Result result;

  if (m_resultDefinition)
  {
    result = this->specification()->createAttribute(m_resultDefinition);

    // Hold on to a copy of the generated result so we can remove it from our
    // specification when the operator is destroyed.
    m_results.push_back(result);
  }
  else
  {
    std::stringstream s;
    s << "Could not identify result attribute definition for operator \"" << this->classname()
      << "\".";
    smtkErrorMacro(this->log(), s.str());
  }

  if (result)
  {
    result->findInt("outcome")->setValue(0, static_cast<int>(outcome));
  }
  return result;
}

void NewOp::generateSummary(NewOp::Result& result)
{
  std::stringstream s;
  int outcome = result->findInt("outcome")->value();
  s << this->classname() << ": ";
  switch (outcome)
  {
    case static_cast<int>(Outcome::UNABLE_TO_OPERATE):
      s << "unable to operate";
      break;
    case static_cast<int>(Outcome::CANCELED):
      s << "operation canceled";
      break;
    case static_cast<int>(Outcome::FAILED):
      s << "operation failed";
      break;
    case static_cast<int>(Outcome::SUCCEEDED):
      s << "operation succeeded";
      break;
    case static_cast<int>(Outcome::UNKNOWN):
      s << "outcome unknown";
      break;
  }

  if (outcome == static_cast<int>(Outcome::SUCCEEDED))
  {
    smtkInfoMacro(this->log(), s.str());
  }
  else
  {
    smtkErrorMacro(this->log(), s.str());
  }
}

NewOp::Specification NewOp::createBaseSpecification() const
{
  Specification spec = smtk::attribute::Collection::create();
  smtk::io::AttributeReader reader;
  reader.readContents(spec, NewOp_xml, this->log());
  return spec;
}

} // operation namespace
} // smtk namespace
