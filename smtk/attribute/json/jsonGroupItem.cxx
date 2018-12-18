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
  if (!n)
  {
    return;
  }

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

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::GroupItemPtr& itemPtr,
  std::vector<ItemExpressionInfo>& itemExpressionInfo, std::vector<AttRefInfo>& attRefInfo)
{
  // The caller should make sure that itemPtr is valid since it's not default constructible
  if (!itemPtr.get())
  {
    return;
  }
  auto temp = smtk::dynamic_pointer_cast<Item>(itemPtr);
  smtk::attribute::from_json(j, temp);

  std::size_t m, n(0);
  n = itemPtr->numberOfGroups();
  m = itemPtr->numberOfItemsPerGroup();
  if (itemPtr->isExtensible())
  {
    // If node has no children, then number of groups is zero
    if (j.is_null())
    {
      n = 0;
    }
    else
    {
      try
      {
        n = j.at("NumberOfGroups");
        itemPtr->setNumberOfGroups(n);
      }
      catch (std::exception& /*e*/)
      {
      }
    }
  }
  if (!n) // There are no sub-groups for this item
  {
    return;
  }
  // There are 2 formats - one is for any number of sub groups and the other
  // is a custon case is for 1 subGroup. They share the same logic here
  json groupClusters;
  try
  {
    groupClusters = j.at("GroupClusters");
  }
  catch (std::exception& /*e*/)
  {
  }
  if (!groupClusters.is_null())
  {
    for (size_t groupIter = 0; groupIter < n; groupIter++)
    {
      std::string clusterName = "Cluster " + std::to_string(groupIter);
      try
      {
        json cluster = groupClusters.at(clusterName);
        if (!cluster.is_null())
        {
          for (size_t itemPGIter = 0; itemPGIter < m; itemPGIter++)
          {
            std::string itemName = "Index " + std::to_string(itemPGIter);
            json itemJson = cluster.at(itemName);
            if (!itemJson.is_null())
            {
              json itemValue = itemJson.at("ItemValue");
              auto groupItemPtr = itemPtr->item(groupIter, itemPGIter);
              smtk::attribute::JsonHelperFunction::processItemTypeFromJson(
                itemValue, groupItemPtr, itemExpressionInfo, attRefInfo);
            }
          }
        }
      }
      catch (std::exception& /*e*/)
      {
      }
    }
  }
}
}
}
