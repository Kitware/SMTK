//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/json/jsonFillOutAttributes.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/task/FillOutAttributes.h"

namespace smtk
{
namespace task
{

void from_json(const nlohmann::json& j, FillOutAttributes::AttributeSet& attributeSet)
{
  auto result = j.find("role");
  if (result != j.end())
  {
    result->get_to(attributeSet.m_role);
  }

  result = j.find("definitions");
  if (result != j.end())
  {
    result->get_to(attributeSet.m_definitions);
  }

  result = j.find("instances");
  if (result != j.end())
  {
    result->get_to(attributeSet.m_instances);
  }

  result = j.find("auto-configure");
  attributeSet.m_autoconfigure = (result != j.end() ? result->get<bool>() : false);

  result = j.find("resource-attributes");
  if (result != j.end())
  {
    result->get_to(attributeSet.m_resources);
  }
}

void to_json(nlohmann::json& j, const FillOutAttributes::AttributeSet& attributeSet)
{
  j = nlohmann::json{ { "role", attributeSet.m_role },
                      { "definitions", attributeSet.m_definitions },
                      { "instances", attributeSet.m_instances },
                      { "resource-attributes", attributeSet.m_resources } };
}

void from_json(const nlohmann::json& j, FillOutAttributes::ResourceAttributes& resourceAttributes)
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

void to_json(nlohmann::json& j, const FillOutAttributes::ResourceAttributes& resourceAttributes)
{
  j = nlohmann::json{ { "valid", resourceAttributes.m_valid },
                      { "invalid", resourceAttributes.m_invalid } };
}

namespace json
{

Task::Configuration jsonFillOutAttributes::operator()(const Task* task, Helper& helper) const
{
  Task::Configuration config;
  auto* nctask = const_cast<Task*>(task);
  auto* fillOutAttributes = dynamic_cast<FillOutAttributes*>(nctask);
  if (fillOutAttributes)
  {
    jsonTask superclass;
    config = superclass(fillOutAttributes, helper);
    nlohmann::json::array_t attributeSets;
    fillOutAttributes->visitAttributeSets(
      [&attributeSets](const FillOutAttributes::AttributeSet& attributeSet) -> smtk::common::Visit {
        nlohmann::json jsonAttributeSet = attributeSet;
        attributeSets.push_back(jsonAttributeSet);
        return smtk::common::Visit::Continue;
      });
    config["attribute-sets"] = attributeSets;
  }
  return config;
}

} // namespace json
} // namespace task
} // namespace smtk
