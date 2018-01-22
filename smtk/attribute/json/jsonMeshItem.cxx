//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonMeshItem.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/json/jsonItem.h"
#include "smtk/common/UUID.h"
#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

using json = nlohmann::json;

/**\brief Provide a way to serialize MeshItemPtr
  */
namespace smtk
{
namespace attribute
{
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::MeshItemPtr& itemPtr)
{
  smtk::attribute::to_json(j, smtk::dynamic_pointer_cast<Item>(itemPtr));
  size_t i = 0, n = itemPtr->numberOfValues();
  // we should always have "NumberOfValues" set
  j["NumberOfValues"] = n;
  if (!n)
  {
    return;
  }

  json values;
  smtk::attribute::MeshItem::const_mesh_it it;
  for (it = itemPtr->begin(); it != itemPtr->end(); ++it, ++i)
  {
    if (itemPtr->isSet(i))
    {
      json value;
      value["Collectionid"] = it->collection()->entity().toString();
      cJSON* jrange = smtk::mesh::to_json(it->range());
      char* cjson = cJSON_Print(jrange);
      cJSON_Delete(jrange);
      free(cjson);
      value["Val"] = cjson;
      values.push_back(value);
    }
  }
  j["Values"] = values;
}

SMTKCORE_EXPORT void from_json(
  const json& j, smtk::attribute::MeshItemPtr& itemPtr, smtk::attribute::CollectionPtr colPtr)
{
  // The caller should make sure that itemPtr is valid since it's not default constructible
  if (!itemPtr.get())
  {
    return;
  }
  auto temp = smtk::dynamic_pointer_cast<Item>(itemPtr);
  smtk::attribute::from_json(j, temp);

  std::size_t i(0), n = itemPtr->numberOfValues();
  smtk::common::UUID cid;
  smtk::model::ManagerPtr modelmgr = colPtr->refModelManager();
  std::size_t numRequiredVals = itemPtr->numberOfRequiredValues();
  if (!numRequiredVals || itemPtr->isExtensible())
  {
    try
    {
      // The node should have an attribute indicating how many values are
      // associated with the item
      n = j.at("NumberOfValues");
      itemPtr->setNumberOfValues(n);
    }
    catch (std::exception& /*e*/)
    {
    }
  }
  if (!n)
  {
    return;
  }
  json values;
  try
  {
    values = j.at("Values");
  }
  catch (std::exception& /*e*/)
  {
  }
  if (!values.is_null())
  {
    for (auto iter = values.begin(); iter != values.end(); iter++, i++)
    {
      json value = *iter;
      std::string collectionId;
      try
      {
        collectionId = value.at("Collectionid");
      }
      catch (std::exception& /*e*/)
      {
      }
      if (collectionId.empty())
      {
        continue;
      }
      if (i >= n)
      {
        break;
      }
      cid = smtk::common::UUID(collectionId);
      //convert back to a handle
      std::string val = value.at("Val");
      cJSON* jshandle = cJSON_Parse(val.c_str());
      smtk::mesh::HandleRange hrange = smtk::mesh::from_json(jshandle);
      cJSON_Delete(jshandle);
      smtk::mesh::CollectionPtr c = modelmgr->meshes()->collection(cid);
      if (!c)
      {
        std::cerr << "Expecting a valid collection for mesh item: " << itemPtr->name() << std::endl;
        continue;
      }
      smtk::mesh::InterfacePtr interface = c->interface();

      if (!interface)
      {
        std::cerr << "Expecting a valid mesh interface for mesh item: " << itemPtr->name()
                  << std::endl;
        continue;
      }

      itemPtr->appendValue(smtk::mesh::MeshSet(c, interface->getRoot(), hrange));
    }
  }
}
}
}
