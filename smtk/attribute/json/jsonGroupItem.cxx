//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonGroupItem.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/json/jsonHelperFunction.h"
#include "smtk/attribute/json/jsonItem.h"
#include "smtk/io/Logger.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize GroupItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::GroupItemPtr& itemPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<Item>(itemPtr));
  size_t m, n;
  std::size_t numRequiredGroups = itemPtr->numberOfRequiredGroups();
  n = itemPtr->numberOfGroups();
  m = itemPtr->numberOfItemsPerGroup();

  // If the group can have variable number of subgroups then store how many
  //  it has
  if (itemPtr->isExtensible())
  {
    j["NumberOfGroups"] = n;
  }

  // Optimize for number of required groups = 1
  else if (numRequiredGroups == 1)
  {
    json groupClusters, cluster;
    for (size_t itemPGIter = 0; itemPGIter < m; itemPGIter++)
    {
      json itemValue, item;
      std::string type = Item::type2String(itemPtr->item(itemPGIter)->type());
      std::string itemName = "Index " + std::to_string(itemPGIter);
      item["Type"] = type;
      smtk::attribute::JsonHelperFunction::processItemTypeToJson(
        itemValue, itemPtr->item(itemPGIter));
      item["ItemValue"] = itemValue;

      // Same type can occur multiple times
      cluster[itemName] = item;
    }
    // Treat it as a group clusters so that from_json can use the some logic
    // for handling result
    groupClusters["Cluster 0"] = cluster;
    j["GroupClusters"] = groupClusters;
    return;
  }

  if (!n)
  {
    return;
  }

  json groupClusters;
  for (size_t groupIter = 0; groupIter < n; groupIter++)
  {
    json cluster;
    std::string clusterName = "Cluster " + std::to_string(groupIter);
    for (size_t itemPGIter = 0; itemPGIter < m; itemPGIter++)
    {
      json itemValue, item;
      std::string type = Item::type2String(itemPtr->item(groupIter, itemPGIter)->type());
      std::string itemName = "Index " + std::to_string(itemPGIter);
      item["Type"] = type;
      smtk::attribute::JsonHelperFunction::processItemTypeToJson(
        itemValue, itemPtr->item(groupIter, itemPGIter));
      item["ItemValue"] = itemValue;

      // Same type can occur multiple times
      cluster[itemName] = item;
    }
    groupClusters[clusterName] = cluster;
  }
  j["GroupClusters"] = groupClusters;
}

SMTKCORE_EXPORT void from_json(
  const json& j,
  smtk::attribute::GroupItemPtr& itemPtr,
  std::vector<ItemExpressionInfo>& itemExpressionInfo,
  std::vector<AttRefInfo>& attRefInfo)
{
  // The caller should make sure that itemPtr is valid since it's not default constructible
  if (!itemPtr.get())
  {
    return;
  }
  std::set<const smtk::attribute::ItemDefinition*> convertedAttDefs;
  auto temp = smtk::dynamic_pointer_cast<Item>(itemPtr);
  smtk::attribute::from_json(j, temp);

  std::size_t m, n(0);
  n = itemPtr->numberOfGroups();
  m = itemPtr->numberOfItemsPerGroup();
  if (itemPtr->isExtensible())
  {
    auto numberOfGroups = j.find("NumberOfGroups");
    if (numberOfGroups != j.end())
    {
      n = *numberOfGroups;
      itemPtr->setNumberOfGroups(n);
    }
    else
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "JSON missing NumberOfGroups "
          << " for extensible group item: " << itemPtr->name());
      return;
    }
  }
  if (!n) // There are no sub-groups for this item
  {
    return;
  }
  // There are 2 formats - one is for any number of sub groups and the other
  // is a custon case is for 1 subGroup. They share the same logic here
  auto groupClusters = j.find("GroupClusters");
  if (groupClusters != j.end())
  {
    for (size_t groupIter = 0; groupIter < n; groupIter++)
    {
      std::string clusterName = "Cluster " + std::to_string(groupIter);
      auto cluster = groupClusters->find(clusterName);
      if (cluster == groupClusters->end())
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "JSON missing: " << clusterName << " for group item: " << itemPtr->name());
        continue;
      }
      for (size_t itemPGIter = 0; itemPGIter < m; itemPGIter++)
      {
        std::string itemName = "Index " + std::to_string(itemPGIter);
        auto itemJson = cluster->find(itemName);
        if (itemJson == cluster->end())
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "JSON missing index: " << itemName << " for cluster: " << clusterName
                                   << " for group item: " << itemPtr->name());
          continue;
        }
        auto itemValue = itemJson->find("ItemValue");
        if (itemValue == itemJson->end())
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "JSON missing ItemValue for index: " << itemName << " for cluster: " << clusterName
                                                 << " for group item: " << itemPtr->name());
          continue;
        }
        auto groupItemPtr = itemPtr->item(groupIter, itemPGIter);
        smtk::attribute::JsonHelperFunction::processItemTypeFromJson(
          *itemValue, groupItemPtr, itemExpressionInfo, attRefInfo, convertedAttDefs);
      }
    }
  }
}
} // namespace attribute
} // namespace smtk
