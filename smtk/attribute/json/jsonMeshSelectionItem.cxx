//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonMeshSelectionItem.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/json/jsonItem.h"
#include "smtk/common/UUID.h"

#include "json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize MeshSelectionItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::MeshSelectionItemPtr& itemPtr)
{
  size_t n = itemPtr->numberOfValues();
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<Item>(itemPtr));
  j["NumberOfValues"] = n;
  j["CtrlKey"] = itemPtr->isCtrlKeyDown();
  j["MeshModifyMode"] = MeshSelectionItem::modifyMode2String(itemPtr->modifyMode());
  if (!n)
  {
    return;
  }
  json selValues;
  smtk::attribute::MeshSelectionItem::const_sel_map_it it;
  for (it = itemPtr->begin(); it != itemPtr->end(); ++it)
  {
    json selV;
    std::string uuid = it->first.toString();
    selV["EntityUUID"] = uuid;
    selV["Values"] = it->second;
    selValues.push_back(selV);
  }
  j["SelectionValues"] = selValues;
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::MeshSelectionItemPtr& itemPtr)
{
  // The caller should make sure that itemPtr is valid since it's not default constructible
  if (!itemPtr.get())
  {
    return;
  }
  auto temp = smtk::dynamic_pointer_cast<Item>(itemPtr);
  smtk::attribute::from_json(j, temp);
  try
  {
    itemPtr->setCtrlKeyDown(j.at("CtrlKey"));
  }
  catch (std::exception& /*e*/)
  {
  }
  try
  {
    itemPtr->setModifyMode(attribute::MeshSelectionItem::string2ModifyMode(j.at("MeshModifyMode")));
  }
  catch (std::exception& /*e*/)
  {
  }
  size_t numberOfV(0);
  try
  {
    numberOfV = j.at("NumberOfValues");
  }
  catch (std::exception& /*e*/)
  {
  }
  if (!numberOfV)
  {
    return;
  }
  json selectionVs;
  try
  {
    selectionVs = j.at("SelectionValues");
  }
  catch (std::exception& /*e*/)
  {
  }
  if (!selectionVs.is_null())
  {
    for (auto iter = selectionVs.begin(); iter != selectionVs.end(); iter++)
    {
      try
      {
        std::set<int> vals = iter->at("Values");
        std::string uuid = iter->at("EntityUUID");
        itemPtr->setValues(smtk::common::UUID(uuid), vals);
      }
      catch (std::exception& /*e*/)
      {
      }
    }
  }
}
}
}
