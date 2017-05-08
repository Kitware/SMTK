//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/SetProperty.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

using namespace smtk::model;
using smtk::attribute::StringItem;
using smtk::attribute::DoubleItem;
using smtk::attribute::IntItem;

namespace smtk
{
namespace model
{

template <typename V, typename VL, typename VD, typename VI>
void SetPropertyValue(const std::string& name, typename VI::Ptr item, EntityRefArray& entities)
{
  EntityRefArray::iterator it;
  if (!item || item->numberOfValues() == 0)
  {
    // Erase the property of this type from these entities,
    // if they had the property in the first place.
    for (it = entities.begin(); it != entities.end(); ++it)
      it->removeProperty<VD>(name);
  }
  else
  {
    // Get the array of values from the item.
    VL values;
    values.reserve(item->numberOfValues());
    for (std::size_t i = 0; i < item->numberOfValues(); ++i)
      values.push_back(item->value(i));

    // Add or overwrite the property with the values.
    for (it = entities.begin(); it != entities.end(); ++it)
      (*it->properties<VD>())[name] = values;
  }
}

template <typename V, typename VL, typename VD, typename VI>
void SetMeshPropertyValue(const std::string& name, typename VI::Ptr item,
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
    (*c->meshProperties<VD>(mesh))[name] = values;
  }
}

smtk::model::OperatorResult SetProperty::operateInternal()
{
  smtk::attribute::StringItemPtr nameItem = this->findString("name");
  smtk::attribute::StringItemPtr stringItem = this->findString("string value");
  smtk::attribute::DoubleItemPtr floatItem = this->findDouble("float value");
  smtk::attribute::IntItemPtr integerItem = this->findInt("integer value");

  EntityRefArray entities = this->associatedEntitiesAs<EntityRefArray>();

  if (nameItem->value(0).empty())
    return this->createResult(smtk::model::OPERATION_FAILED);

  SetPropertyValue<String, StringList, StringData, StringItem>(
    nameItem->value(0), stringItem, entities);
  SetPropertyValue<Float, FloatList, FloatData, DoubleItem>(
    nameItem->value(0), floatItem, entities);
  SetPropertyValue<Integer, IntegerList, IntegerData, IntItem>(
    nameItem->value(0), integerItem, entities);

  // check whether there are mesh entities's properties need to be changed
  smtk::attribute::MeshItemPtr meshItem = this->findMesh("meshes");
  smtk::mesh::MeshSets modifiedMeshes;
  smtk::model::EntityRefs extraModifiedModels;
  if (meshItem)
  {
    smtk::attribute::MeshItem::const_mesh_it it;
    for (it = meshItem->begin(); it != meshItem->end(); ++it)
    {
      smtk::mesh::CollectionPtr c = it->collection();
      if (!c)
        continue;
      SetMeshPropertyValue<String, StringList, StringData, StringItem>(
        nameItem->value(0), stringItem, c, *it);
      SetMeshPropertyValue<Float, FloatList, FloatData, DoubleItem>(
        nameItem->value(0), floatItem, c, *it);
      SetMeshPropertyValue<Integer, IntegerList, IntegerData, IntItem>(
        nameItem->value(0), integerItem, c, *it);
      modifiedMeshes.insert(*it);

      // label the associated model as modified
      smtk::common::UUID modid = c->associatedModel();
      if (!modid.isNull())
        extraModifiedModels.insert(smtk::model::Model(this->manager(), modid));
    }
  }

  smtk::model::OperatorResult result = this->createResult(smtk::model::OPERATION_SUCCEEDED);

  // if a model is in the changed entities and it is a submodel, we
  // want to label its parent model to be modified too.
  smtk::model::EntityRefArray::iterator it;
  for (it = entities.begin(); it != entities.end(); ++it)
  {
    if (it->isModel() && it->as<model::Model>().parent().isModel())
    {
      smtk::model::Model pmodel = it->as<model::Model>().parent().as<model::Model>();
      extraModifiedModels.insert(pmodel);
    }
  }

  entities.insert(entities.end(), extraModifiedModels.begin(), extraModifiedModels.end());

  // Return the list of entities that were potentially
  // modified so that remote sessions can track what records
  // need to be re-fetched.
  smtk::attribute::ModelEntityItem::Ptr resultEntities = result->findModelEntity("modified");
  resultEntities->setValues(entities.begin(), entities.end());

  // Return the list of meshes that were potentially modified.
  if (modifiedMeshes.size() > 0)
  {
    smtk::attribute::MeshItemPtr resultMeshes = result->findMesh("mesh_modified");
    if (resultMeshes)
      resultMeshes->appendValues(modifiedMeshes);
  }

  return result;
}

} //namespace model
} // namespace smtk

#include "smtk/model/SetProperty_xml.h"

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::model::SetProperty, set_property, "set property",
  SetProperty_xml, smtk::model::Session);
