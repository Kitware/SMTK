//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/json/jsonFillOutAttributesAgent.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/task/Manager.h"
#include "smtk/task/Port.h"

namespace smtk
{
namespace task
{

void from_json(const nlohmann::json& jj, FillOutAttributesAgent::AttributeSet& attributeSet)
{
  auto result = jj.find("role");
  if (result != jj.end())
  {
    result->get_to(attributeSet.m_role);
  }

  result = jj.find("definitions");
  if (result != jj.end())
  {
    result->get_to(attributeSet.m_definitions);
  }

  result = jj.find("instances");
  if (result != jj.end())
  {
    result->get_to(attributeSet.m_instances);
  }

  result = jj.find("auto-configure");
  attributeSet.m_autoconfigure = (result != jj.end() ? result->get<bool>() : false);

  result = jj.find("resource-attributes");
  if (result != jj.end())
  {
    result->get_to(attributeSet.m_resources);
  }

  result = jj.find("output-data");
  if (result != jj.end())
  {
    attributeSet.m_outputData =
      FillOutAttributesAgent::PortDataObjectsValue(result->get<smtk::string::Token>());
  }
  else
  {
    attributeSet.m_outputData = FillOutAttributesAgent::PortDataObjects::Resources;
  }
}

void to_json(nlohmann::json& jj, const FillOutAttributesAgent::AttributeSet& attributeSet)
{
  jj = nlohmann::json{ { "role", attributeSet.m_role },
                       { "definitions", attributeSet.m_definitions },
                       { "instances", attributeSet.m_instances },
                       { "resource-attributes", attributeSet.m_resources } };
}

void from_json(
  const nlohmann::json& jj,
  FillOutAttributesAgent::ResourceAttributes& resourceAttributes)
{
  auto result = jj.find("valid");
  if (result != jj.end())
  {
    result->get_to(resourceAttributes.m_valid);
  }
  result = jj.find("invalid");
  if (result != jj.end())
  {
    result->get_to(resourceAttributes.m_invalid);
  }
}

void to_json(
  nlohmann::json& jj,
  const FillOutAttributesAgent::ResourceAttributes& resourceAttributes)
{
  jj = nlohmann::json{ { "valid", resourceAttributes.m_valid },
                       { "invalid", resourceAttributes.m_invalid } };
}

} // namespace task
} // namespace smtk
