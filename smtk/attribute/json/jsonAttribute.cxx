//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonAttribute.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"

#include "smtk/attribute/json/jsonHelperFunction.h"
#include "smtk/attribute/json/jsonItem.h"
#include "smtk/common/UUID.h"
#include "smtk/io/Logger.h"
#include "smtk/model/Resource.h"

#include "smtk/PublicPointerDefs.h"

#include "nlohmann/json.hpp"

#include <iosfwd>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize valueItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::AttributePtr& att)
{ // Follow the logic in XmlV2StringWriter::processAttribute
  j["Name"] = att->name();
  if (att->definition())
  {
    j["Type"] = att->type();
    if (att->definition()->isNodal())
    {
      j["OnInteriorNodes"] = att->appliesToInteriorNodes();
      j["OnBoundaryNodes"] = att->appliesToBoundaryNodes();
    }
  }
  j["ID"] = att->id().toString();

  // Save associated entities
  auto assoc = att->associations();
  if (assoc && assoc->numberOfValues() > 0)
  {
    j["Associations"] = assoc;
  }
  // Save Color Information
  if (att->isColorSet())
  {
    const double* rgba = att->color();
    std::vector<double> rgbaV;
    rgbaV.push_back(rgba[0]);
    rgbaV.push_back(rgba[1]);
    rgbaV.push_back(rgba[2]);
    rgbaV.push_back(rgba[3]);
    j["Color"] = rgbaV;
  }
  // Does the Attribute have explicit advance level information
  if (att->hasLocalAdvanceLevelInfo(0))
  {
    j["AdvanceReadLevel"] = att->localAdvanceLevel(0);
  }

  if (att->hasLocalAdvanceLevelInfo(1))
  {
    j["AdvanceWriteLevel"] = att->localAdvanceLevel(1);
  }

  if (!att->localUnits().empty())
  {
    j["Units"] = att->localUnits();
  }

  // Process its Items
  int i, n = static_cast<int>(att->numberOfItems());
  if (n)
  {
    json items = json::array();
    for (i = 0; i < n; i++)
    {
      json item;
      // TODO Add name check and whether json equals the item of the attribute
      smtk::attribute::JsonHelperFunction::processItemTypeToJson(item, att->item(i));
      // Same type items can occur multiple times
      items.push_back(item);
    }
    j["Items"] = items;
  }
}

SMTKCORE_EXPORT void from_json(
  const json& j,
  smtk::attribute::AttributePtr& att,
  std::vector<smtk::attribute::ItemExpressionInfo>& itemExpressionInfo,
  std::vector<smtk::attribute::AttRefInfo>& attRefInfo,
  const std::set<const smtk::attribute::ItemDefinition*>& convertedAttDefs)
{ // Follow the logic in XmlDocV1Parser::processAttribute::L1753
  auto result = j.find("OnInteriorNodes");
  if (result != j.end())
  {
    att->setAppliesToInteriorNodes(*result);
  }

  result = j.find("OnBoundaryNodes");
  if (result != j.end())
  {
    att->setAppliesToBoundaryNodes(*result);
  }

  result = j.find("Color");
  if ((result != j.end()) && (result->size() == 4))
  {
    double color[4];
    color[0] = (*result)[0];
    color[1] = (*result)[1];
    color[2] = (*result)[2];
    color[3] = (*result)[3];
    att->setColor(color);
  }

  // Process local advance level info
  result = j.find("AdvanceReadLevel");
  if (result != j.end())
  {
    att->setLocalAdvanceLevel(0, *result);
  }
  result = j.find("AdvanceWriteLevel");
  if (result != j.end())
  {
    att->setLocalAdvanceLevel(1, *result);
  }

  result = j.find("Units");
  if (result != j.end())
  {
    att->setLocalUnits(*result);
  }

  // Process items
  result = j.find("Items");
  if (result != j.end())
  {
    for (const auto& item : *result)
    {
      auto itemName = item.find("Name");
      if (itemName == item.end())
      {
        smtkErrorMacro(smtk::io::Logger::instance(), "JSON missing attribute Item Name");
        continue;
      }
      auto attItem = att->find(*itemName);
      if (attItem == nullptr)
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(), "Could not find attribute Item  with name:" << *itemName);
        continue;
      }
      smtk::attribute::JsonHelperFunction::processItemTypeFromJson(
        item, attItem, itemExpressionInfo, attRefInfo, convertedAttDefs);
    }
  }
  // Process associations
  result = j.find("Associations");
  if (result != j.end())
  {
    auto assocItem = att->associations();
    smtk::attribute::from_json(*result, assocItem, itemExpressionInfo, attRefInfo);
  }
}
} // namespace attribute
} // namespace smtk
