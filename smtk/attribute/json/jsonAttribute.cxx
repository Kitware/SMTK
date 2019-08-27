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

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::AttributePtr& att,
  std::vector<smtk::attribute::ItemExpressionInfo>& itemExpressionInfo,
  std::vector<smtk::attribute::AttRefInfo>& attRefInfo,
  const std::set<const smtk::attribute::ItemDefinition*>& convertedAttDefs)
{ // Follow the logic in XmlDocV1Parser::processAttribute::L1753
  try
  {
    att->setAppliesToInteriorNodes(j.at("OnInteriorNodes"));
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    att->setAppliesToBoundaryNodes(j.at("OnBoundaryNodes"));
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    std::vector<double> colorV = j.at("Color");
    double color[4];
    for (int i = 0; i < 4; i++)
    {
      color[i] = colorV[i];
    }
    att->setColor(color);
  }
  catch (std::exception& /*e*/)
  {
  }

  try
  {
    std::vector<double> colorV = j.at("Color");
    double color[4];
    for (int i = 0; i < 4; i++)
    {
      color[i] = colorV[i];
    }
    att->setColor(color);
  }
  catch (std::exception& /*e*/)
  {
  }
  // Process items
  json items;
  try
  {
    items = j.at("Items");
  }
  catch (std::exception& /*e*/)
  {
  }
  if (!items.is_null())
  {
    int i = 0;
    for (auto itemIter = items.begin(); itemIter != items.end(); itemIter++, i++)
    {
      try
      {
        auto itemToProcess = att->find(itemIter->at("Name"));
        if (!itemToProcess)
        {
          continue;
        }
        smtk::attribute::JsonHelperFunction::processItemTypeFromJson(
          *itemIter, itemToProcess, itemExpressionInfo, attRefInfo, convertedAttDefs);
      }
      catch (std::exception& /*e*/)
      {
      }
    }
  }
  // Process items
  json association;
  try
  {
    association = j.at("Associations");
  }
  catch (std::exception& /*e*/)
  {
  }
  if (!association.is_null())
  {
    auto assocItem = att->associations();
    smtk::attribute::from_json(association, assocItem);
  }
}
}
}
