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

#include "smtk/attribute/Resource.h"
#include "smtk/io/Logger.h"

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
  std::cout << __FILE__ << ":" << __LINE__ << " " << std::endl;

  // Input task - must be FillOutAttributes
  auto* fromTask = dynamic_cast<FillOutAttributes*>(this->from());
  if (fromTask == nullptr)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "ConfigureOperation \"from\" task is not type FillOutAttributes.");
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
        smtk::io::Logger::instance(),
        "ConfigureOperation failed to find matching role \"" << paramSet.m_fromRole << "\"");
    }

    std::cout << __FILE__ << ":" << __LINE__ << " " << foundMatch << std::endl;
  }

  return false;
}

void ConfigureOperation::configureSelf(const Configuration& config)
{
  auto configIter = config.find("configure");
  if (configIter == config.end())
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(), "ConfigureOperation adaptor missing \"configure\" element.");
    return;
  }
  else if (!configIter->is_array())
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(), "ConfigureOperation \"configure\" element is not an array.");
    return;
  }

  for (auto it = configIter->begin(); it != configIter->end(); ++it)
  {
    if (!it->is_object())
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "ConfigureOperation \"configure\" has element that is not an object.");
      continue;
    }

    ParameterSet paramSet;

    // Traverse all items in the object
    for (auto& el : it->items())
    {
      std::string key = el.key();
      std::string value = el.value().get<std::string>();
      if (key == "from-role")
      {
        paramSet.m_fromRole = value;
      }
      else
      {
        paramSet.m_pathMap[key] == value;
      }
    } // for (el)

    if (paramSet.m_fromRole.empty())
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "ConfigureOperation attribute-set missing \"from-role\" item.");
    }
    else
    {
      m_parameterSets.push_back(paramSet);
    }
  } // for (configIter)
}

void ConfigureOperation::updateOperation(
  const smtk::task::FillOutAttributes::AttributeSet& attSet,
  const ParameterSet& paramSet)
{
  std::cout << __FILE__ << ":" << __LINE__ << " " << std::endl;
}

} // namespace adaptor
} // namespace task
} // namespace smtk
