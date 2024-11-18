//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/json/jsonSubmitOperationAgent.h"

#include "smtk/task/SubmitOperationAgent.h"
#include "smtk/task/Task.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/string/json/jsonToken.h"

namespace smtk
{
namespace task
{

void from_json(const nlohmann::json& jj, SubmitOperationAgent::ParameterSpec& parameterSpec)
{
  parameterSpec.m_portDataHandler = SubmitOperationAgent::PortDataHandlerValue(jj.at("handler"));
  parameterSpec.m_resourceTypeName = jj.at("resource-type").get<std::string>();
  if (jj.contains("resource-template"))
  {
    parameterSpec.m_resourceTemplate = jj.at("resource-template").get<smtk::string::Token>();
  }
  parameterSpec.m_componentSelector = jj.at("component-selector").get<std::string>();
  if (jj.contains("source-path"))
  {
    // The source-path is only required if the port-data handler is set
    // to assign attribute items to the parameter.
    parameterSpec.m_sourceItemPath = jj.at("source-path").get<std::string>();
  }
  parameterSpec.m_targetItemPath = jj.at("target-path").get<std::string>();
  auto it = jj.find("configured-by");
  if (it != jj.end())
  {
    parameterSpec.m_configuredBy =
      SubmitOperationAgent::ConfiguredByValue(it->get<smtk::string::Token>());
  }
  if (jj.contains("user-override"))
  {
    parameterSpec.m_userOverride = jj.at("user-override").get<bool>();
  }

  it = jj.find("visibility");
  if (it != jj.end())
  {
    parameterSpec.m_visibility =
      SubmitOperationAgent::ItemVisibilityValue(it->get<smtk::string::Token>());
  }
}

void to_json(nlohmann::json& jj, const SubmitOperationAgent::ParameterSpec& parameterSpec)
{
  // clang-format off
  jj = nlohmann::json{
    { "handler", SubmitOperationAgent::PortDataHandlerToken(parameterSpec.m_portDataHandler) },
    { "resource-type", parameterSpec.m_resourceTypeName },
    { "component-selector", parameterSpec.m_componentSelector },
    { "source-path", parameterSpec.m_sourceItemPath },
    { "target-path", parameterSpec.m_targetItemPath },
    { "configured-by", SubmitOperationAgent::ConfiguredByToken(parameterSpec.m_configuredBy) }
  };
  // clang-format on
  if (parameterSpec.m_resourceTemplate.valid())
  {
    jj["resource-template"] = parameterSpec.m_resourceTemplate;
  }
  if (parameterSpec.m_userOverride)
  {
    jj["user-override"] = true;
  }
  switch (parameterSpec.m_configuredBy)
  {
    case SubmitOperationAgent::ConfiguredBy::Static:
    case SubmitOperationAgent::ConfiguredBy::Port:
      if (parameterSpec.m_visibility != SubmitOperationAgent::ItemVisibility::RecursiveOff)
      {
        jj["visibility"] = SubmitOperationAgent::ItemVisibilityToken(parameterSpec.m_visibility);
      }
      break;
    default:
    case SubmitOperationAgent::ConfiguredBy::User:
      if (parameterSpec.m_visibility != SubmitOperationAgent::ItemVisibility::On)
      {
        jj["visibility"] = SubmitOperationAgent::ItemVisibilityToken(parameterSpec.m_visibility);
      }
      break;
  }
}

void from_json(const nlohmann::json& jj, SubmitOperationAgent::ParameterSpecRef& specRef)
{
  specRef.m_portName = jj.at("port").get<smtk::string::Token>();
  specRef.m_roleName = jj.at("role").get<smtk::string::Token>();
  specRef.m_specIndex = jj.at("idx").get<std::size_t>();
  specRef.m_expunge = jj.contains("expunge") && jj.at("expunge").get<bool>();
}

void to_json(nlohmann::json& jj, const SubmitOperationAgent::ParameterSpecRef& specRef)
{
  jj["port"] = specRef.m_portName;
  jj["role"] = specRef.m_roleName;
  jj["idx"] = specRef.m_specIndex;
  if (specRef.m_expunge)
  {
    jj["expunge"] = true;
  }
}

} // namespace task
} // namespace smtk
