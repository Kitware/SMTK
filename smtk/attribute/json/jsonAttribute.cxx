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
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/attribute/json/jsonHelperFunction.h"
#include "smtk/attribute/json/jsonItem.h"
#include "smtk/attribute/json/jsonModelEntityItem.h"
#include "smtk/io/Logger.h"
#include "smtk/model/Manager.h"

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
  smtk::attribute::ModelEntityItemPtr assoc = att->associations();
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
      json item, itemValue;
      item["Type"] = Item::type2String(att->item(i)->type());
      // TODO Add name check and whether json equals the item of the attribute
      smtk::attribute::JsonHelperFunction::processItemTypeToJson(itemValue, att->item(i));
      // Same type items can occur multiple times
      item["ItemValue"] = itemValue;
      items.push_back(item);
    }
    j["Items"] = items;
  }
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::AttributePtr& att,
  const smtk::attribute::CollectionPtr& colPtr,
  std::vector<smtk::attribute::ItemExpressionInfo>& itemExpressionInfo,
  std::vector<smtk::attribute::AttRefInfo>& attRefInfo)
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
        json itemJson = itemIter->at("ItemValue");
        auto itemToProcess = att->item(i);
        smtk::attribute::JsonHelperFunction::processItemTypeFromJson(
          itemJson, itemToProcess, colPtr, itemExpressionInfo, attRefInfo);
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
    smtk::attribute::ModelEntityItem::Ptr assocsItem = att->associations();
    smtk::attribute::from_json(association, assocsItem, colPtr);
    // Now the ModelEntityItem is deserialized but we need
    // to let the model manager know about the associations
    // (assuming we have a model manager):
    smtk::model::Manager::Ptr mmgr = att->modelManager();
    if (mmgr)
    {
      smtk::attribute::ModelEntityItem::const_iterator eit;
      for (eit = assocsItem->begin(); eit != assocsItem->end(); ++eit)
      {
        // Calling associateAttribute() with a NULL attribute collection
        // prevents the model manager from attempting to add the association
        // back to the attribute we are currently deserializing:
        mmgr->associateAttribute(NULL, att->id(), eit->entity());
      }
    }
  }
}
}
}
