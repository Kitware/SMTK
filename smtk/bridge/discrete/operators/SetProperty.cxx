//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/discrete/operators/SetProperty.h"

#include "smtk/bridge/discrete/Session.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "smtk/mesh/Manager.h"
#include "smtk/mesh/Collection.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/discrete/kernel/Model/vtkModelEntity.h"
#include "smtk/bridge/discrete/kernel/vtkModelUserName.h"

using namespace smtk::model;
using smtk::attribute::StringItem;
using smtk::attribute::DoubleItem;
using smtk::attribute::IntItem;

namespace smtk {
  namespace bridge {
    namespace discrete {

template<typename V, typename VL, typename VD, typename VI>
void SetProperty::setPropertyValue(const std::string& pname, typename VI::Ptr item, smtk::model::EntityRefArray& entities)
{
  smtk::model::EntityRefArray::iterator it;
  if (!item || item->numberOfValues() == 0)
    {
    // Erase the property of this type from these entities,
    // if they had the property in the first place.
    for (it = entities.begin(); it != entities.end(); ++it)
      {
      it->removeProperty<VD>(pname);
      }
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
      (*it->properties<VD>())[pname] = values;
    }
}

template<typename V, typename VL, typename VD, typename VI>
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

void SetProperty::setName(const std::string& pname, smtk::model::EntityRefArray& entities)
{
  smtk::model::EntityRefArray::iterator it;
  for (it = entities.begin(); it != entities.end(); ++it)
    {
    vtkModelEntity* discEnt = this->discreteEntityAs<vtkModelEntity*>(*it);
    if (discEnt)
      vtkModelUserName::SetUserName(discEnt, pname.empty() ? NULL : pname.c_str());
    }
}

void SetProperty::setColor(smtk::attribute::DoubleItemPtr color, smtk::model::EntityRefArray& entities)
{
  double rgba[] = {-1, -1, -1, 1};
  std::size_t nc = static_cast<std::size_t>(color->numberOfValues());
  if (nc > 4)
    nc = 4;

  for (std::size_t i = 0; i < nc; ++i)
    rgba[i] = color->value(i);

  smtk::model::EntityRefArray::iterator it;
  for (it = entities.begin(); it != entities.end(); ++it)
    {
    vtkModelEntity* discEnt = this->discreteEntityAs<vtkModelEntity*>(*it);
    if (discEnt)
      discEnt->SetColor(rgba[0], rgba[1], rgba[2], rgba[3]);
    }
}

void SetProperty::setVisibility(int visibility, smtk::model::EntityRefArray& entities)
{
  smtk::model::EntityRefArray::iterator it;
  for (it = entities.begin(); it != entities.end(); ++it)
    {
    vtkModelEntity* discEnt = this->discreteEntityAs<vtkModelEntity*>(*it);
    if (discEnt)
      discEnt->SetVisibility(visibility);
    }
}

smtk::model::OperatorResult SetProperty::operateInternal()
{
  smtk::attribute::StringItemPtr nameItem = this->findString("name");
  smtk::attribute::StringItemPtr stringItem = this->findString("string value");
  smtk::attribute::DoubleItemPtr floatItem = this->findDouble("float value");
  smtk::attribute::IntItemPtr integerItem = this->findInt("integer value");

  smtk::model::EntityRefArray entities = this->associatedEntitiesAs<smtk::model::EntityRefArray>();

  if (nameItem->value(0).empty())
    return this->createResult(smtk::model::OPERATION_FAILED);

  std::string propName = nameItem->value(0);
  this->setPropertyValue<String,StringList,StringData,StringItem>(
    propName, stringItem, entities);
  this->setPropertyValue<Float,FloatList,FloatData,DoubleItem>(
    propName, floatItem, entities);
  this->setPropertyValue<Integer,IntegerList,IntegerData,IntItem>(
    propName, integerItem, entities);

  if (propName == "name" && stringItem->numberOfValues() > 0)
    this->setName(stringItem->value(0), entities);
  else if (propName == "color")
    this->setColor(floatItem, entities);
  else if (propName == "visible" && integerItem->numberOfValues() > 0)
    this->setVisibility(integerItem->value(0), entities);

  // check whether there are mesh entities's properties need to be changed
  smtk::attribute::MeshItemPtr meshItem = this->findMesh("meshes");
  smtk::mesh::MeshSets modifiedMeshes;
  smtk::model::EntityRefs extraModifiedModels;
  if(meshItem)
    {
    smtk::model::ManagerPtr modelmgr = this->manager();
    smtk::mesh::ManagerPtr meshmgr = modelmgr->meshes();
    smtk::attribute::MeshItem::const_mesh_it it;
    for(it = meshItem->begin(); it != meshItem->end(); ++it)
      {
      smtk::mesh::CollectionPtr c = it->collection();
      if(!c)
        continue;
      SetMeshPropertyValue<String,StringList,StringData,StringItem>(
        nameItem->value(0), stringItem, c, *it);
      SetMeshPropertyValue<Float,FloatList,FloatData,DoubleItem>(
        nameItem->value(0), floatItem, c, *it);
      SetMeshPropertyValue<Integer,IntegerList,IntegerData,IntItem>(
        nameItem->value(0), integerItem, c, *it);
      modifiedMeshes.insert(*it);
  
      // label the assoicated model as modified
      smtk::common::UUID modid = c->associatedModel();
    if(!modid.isNull())
        extraModifiedModels.insert(smtk::model::Model(this->manager(), modid));
      }
    }

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);

  // if a model is in the changed entities and it is a submodel, we
  // want to label its parent model to be modified too.
  smtk::model::EntityRefArray::iterator it;
  for (it = entities.begin(); it != entities.end(); ++it)
    {
    if(it->isModel() && it->as<model::Model>().parent().isModel())
      {
      smtk::model::Model pmodel = it->as<model::Model>().parent().as<model::Model>();
      extraModifiedModels.insert(pmodel);
      }
    }

  entities.insert(entities.end(), extraModifiedModels.begin(), extraModifiedModels.end());

  // Return the list of entities that were potentially
  // modified so that remote sessions can track what records
  // need to be re-fetched.
  smtk::attribute::ModelEntityItem::Ptr resultEntities =
    result->findModelEntity("modified");
  resultEntities->setValues(entities.begin(), entities.end());

  // Return the list of meshes that were potentially modified.
  if (modifiedMeshes.size() > 0)
    {
    smtk::attribute::MeshItemPtr resultMeshes =
      result->findMesh("mesh_modified");
    if(resultMeshes)
      resultMeshes->appendValues(modifiedMeshes);
    }

  return result;
}

    } //namespace discrete
  } //namespace bridge
} // namespace smtk

#include "smtk/model/SetProperty_xml.h"

smtkImplementsModelOperator(
  SMTKDISCRETESESSION_EXPORT,
  smtk::bridge::discrete::SetProperty,
  discrete_set_property,
  "set property",
  SetProperty_xml,
  smtk::bridge::discrete::Session);
