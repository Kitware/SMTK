//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/json/jsonSubmitOperation.h"

#include "smtk/task/SubmitOperation.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/string/json/jsonToken.h"

namespace smtk
{
namespace task
{

void from_json(const nlohmann::json& j, SubmitOperation::ParameterSpec& parameterSpec)
{
  auto it = j.find("enable");
  if (it != j.end())
  {
    it->get_to(parameterSpec.m_enable);
  }

  /*
  it = j.find("value");
  if (it != j.end())
  {
    it->get_to(parameterSpec.m_values);
  }
  */

  it = j.find("visibility");
  if (it != j.end())
  {
    parameterSpec.m_visibility =
      SubmitOperation::ItemVisibilityValue(it->get<smtk::string::Token>());
  }

  it = j.find("role");
  if (it != j.end())
  {
    it->get_to(parameterSpec.m_role);
  }

  it = j.find("configured-by");
  if (it != j.end())
  {
    parameterSpec.m_configuredBy =
      SubmitOperation::ConfiguredByValue(it->get<smtk::string::Token>());
  }
}

void to_json(nlohmann::json& j, const SubmitOperation::ParameterSpec& parameterSpec)
{
  j = nlohmann::json{ { "configured-by",
                        SubmitOperation::ConfiguredByToken(parameterSpec.m_configuredBy) },
                      { "enabled", parameterSpec.m_enable } };
  if (!parameterSpec.m_values.empty())
  {
    j["value"] = parameterSpec.m_values;
  }
  if (!parameterSpec.m_role.empty())
  {
    j["role"] = parameterSpec.m_role;
  }
  switch (parameterSpec.m_configuredBy)
  {
    case SubmitOperation::ConfiguredBy::Task:
    case SubmitOperation::ConfiguredBy::Adaptor:
      if (parameterSpec.m_visibility != SubmitOperation::ItemVisibility::RecursiveOff)
      {
        j["visibility"] = SubmitOperation::ItemVisibilityToken(parameterSpec.m_visibility);
      }
      break;
    default:
    case SubmitOperation::ConfiguredBy::User:
      if (parameterSpec.m_visibility != SubmitOperation::ItemVisibility::On)
      {
        j["visibility"] = SubmitOperation::ItemVisibilityToken(parameterSpec.m_visibility);
      }
      break;
  }
}

#if 0
void from_json(const nlohmann::json& j, SubmitOperation::ResourceAttributes& resourceAttributes)
{
  auto result = j.find("valid");
  if (result != j.end())
  {
    result->get_to(resourceAttributes.m_valid);
  }
  result = j.find("invalid");
  if (result != j.end())
  {
    result->get_to(resourceAttributes.m_invalid);
  }
}

void to_json(nlohmann::json& j, const SubmitOperation::ResourceAttributes& resourceAttributes)
{
  j = nlohmann::json{ { "valid", resourceAttributes.m_valid },
                      { "invalid", resourceAttributes.m_invalid } };
}
#endif // 0

namespace json
{

Task::Configuration jsonSubmitOperation::operator()(const Task* task, Helper& helper) const
{
  Task::Configuration config;
  auto* nctask = const_cast<Task*>(task);
  auto* submitOperation = dynamic_cast<SubmitOperation*>(nctask);
  if (submitOperation)
  {
    jsonTask superclass;
    config = superclass(submitOperation, helper);
    if (auto* op = submitOperation->operation())
    {
      config["operation"] = op->typeName();
      // TODO: Save all parameters?
    }
    config["run-style"] = SubmitOperation::RunStyleToken(submitOperation->runStyle());
    config["run-since-edited"] = submitOperation->runSinceEdited();
    nlohmann::json::array_t parameterSpecs;
    submitOperation->visitParameterSpecs(
      [&parameterSpecs](
        const SubmitOperation::ParameterSpec& parameterSpec) -> smtk::common::Visit {
        nlohmann::json jsonParameterSpec = parameterSpec;
        parameterSpecs.push_back(jsonParameterSpec);
        return smtk::common::Visit::Continue;
      });
    config["parameters"] = parameterSpecs;
    auto assocParam = submitOperation->associationSpec();
    if (assocParam.m_itemPath != "-ignore-")
    {
      config["associations"] = assocParam;
    }
  }
  return config;
}

} // namespace json
} // namespace task
} // namespace smtk
