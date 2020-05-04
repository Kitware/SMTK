//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/workflow/json/jsonOperationFilterSort.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Metadata.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace workflow
{

void to_json(json& j, const OperationFilterSortPtr& ofs, smtk::operation::Manager::Ptr manager)
{
  if (!manager)
  {
    smtkWarningMacro(smtk::io::Logger::instance(),
      "No operation manager available to serialize OperationFilterSort.");
    return;
  }
  const auto& meta = manager->metadata().get<smtk::operation::IndexTag>();
  json jflist = json::array();
  const auto& filterList = ofs->filterList();
  for (const auto& entry : filterList)
  {
    // Entries must have an index, name, and precedence:
    json jentry = {
      { "operation", meta.find(entry.first)->typeName() }, { "name", entry.second.name },
      { "precedence", entry.second.precedence },
    };
    // The description and iconName fields are optional:
    if (!entry.second.description.empty())
    {
      jentry["description"] = entry.second.description;
    }
    if (!entry.second.iconName.empty())
    {
      jentry["iconName"] = entry.second.iconName;
    }
    jflist.push_back(jentry);
  }
  j = {
    { "filterList", jflist },
  };
}

void from_json(const json& j, OperationFilterSortPtr& ofs, smtk::operation::ManagerPtr manager)
{
  ofs = OperationFilterSort::create();
  if (!manager)
  {
    smtkWarningMacro(smtk::io::Logger::instance(),
      "No operation manager available to deserialize OperationFilterSort.");
    return;
  }
  const auto& meta = manager->metadata().get<smtk::operation::NameTag>();
  OperationFilterSort::FilterList& filterList = ofs->filterList();
  for (auto entry : j.at("filterList"))
  {
    auto& filter = filterList[meta.find(entry.at("operation"))->index()];
    filter.name = entry.at("name").get<std::string>();
    if (entry.find("description") != entry.end())
    {
      filter.description = entry.at("description").get<std::string>();
    }
    if (entry.find("iconName") != entry.end())
    {
      filter.iconName = entry.at("iconName").get<std::string>();
    }
    if (entry.find("precedence") != entry.end())
    {
      filter.precedence = entry.at("precedence");
    }
  }
}
}
}
