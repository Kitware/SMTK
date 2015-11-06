//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/operators/SetProperty.h"

#include "smtk/model/Session.h"

#include "smtk/model/Manager.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/Collection.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

using namespace smtk::model;
using smtk::attribute::StringItem;
using smtk::attribute::DoubleItem;
using smtk::attribute::IntItem;

namespace smtk {
  namespace mesh {

template<typename V, typename VL, typename VD, typename VI>
void SetPropertyValue(const std::string& name, typename VI::Ptr item,
  smtk::mesh::CollectionPtr c, const smtk::mesh::MeshSet& mesh)
{
  EntityRefArray::iterator it;
  if (!item || item->numberOfValues() == 0)
    {
    // Erase the property of this type from these entities,
    // if they had the property in the first place.
    c->removeProperty<VD>(mesh, name);
    }
  else
    {
    // Get the array of values from the item.
    VL values;
    values.reserve(item->numberOfValues());
    for (std::size_t i = 0; i < item->numberOfValues(); ++i)
      values.push_back(item->value(i));

    // Add or overwrite the property with the values.
    (*c->properties<VD>(mesh))[name] = values;
    }
}

smtk::model::OperatorResult SetProperty::operateInternal()
{
  smtk::attribute::StringItemPtr nameItem = this->findString("name");
  smtk::attribute::StringItemPtr stringItem = this->findString("string value");
  smtk::attribute::DoubleItemPtr floatItem = this->findDouble("float value");
  smtk::attribute::IntItemPtr integerItem = this->findInt("integer value");
  smtk::attribute::MeshItemPtr meshItem = this->findMesh("meshes");

  std::string propname = nameItem->value(0);
  if (propname.empty())
    return this->createResult(smtk::model::OPERATION_FAILED);

  std::map<smtk::common::UUID, smtk::mesh::MeshSet > modifiedMeshes;
  smtk::model::ManagerPtr modelmgr = this->manager();
  smtk::mesh::ManagerPtr meshmgr = modelmgr->meshes();
  smtk::attribute::MeshItem::const_mesh_it it;
  for(it = meshItem->begin(); it != meshItem->end(); ++it)
    {
    smtk::mesh::CollectionPtr c = meshmgr->collection(it->first);
    if(c->isValid())
      {
      SetPropertyValue<String,StringList,StringData,StringItem>(
        propname, stringItem, c, it->second);
      SetPropertyValue<Float,FloatList,FloatData,DoubleItem>(
        propname, floatItem, c, it->second);
      SetPropertyValue<Integer,IntegerList,IntegerData,IntItem>(
        propname, integerItem, c, it->second);
      modifiedMeshes[c->entity()] = it->second;
      }
    }

  if (modifiedMeshes.size() == 0)
    return this->createResult(smtk::model::OPERATION_FAILED);

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);

  // Return the list of meshes that were potentially
  // modified so that remote sessions can track what records
  // need to be re-fetched.
  smtk::attribute::MeshItemPtr resultMeshes =
    result->findMesh("mesh_modified");
  for(it = modifiedMeshes.begin(); it != modifiedMeshes.end(); ++it)
    resultMeshes->setValue(it->first, it->second);

  return result;
}

  } //namespace mesh
} // namespace smtk

#include "smtk/mesh/SetProperty_xml.h"

smtkImplementsModelOperator(
  SMTKCORE_EXPORT,
  smtk::mesh::SetProperty,
  set_mesh_property,
  "set mesh property",
  SetProperty_xml,
  smtk::model::Session);
