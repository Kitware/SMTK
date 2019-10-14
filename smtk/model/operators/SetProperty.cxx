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
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Session.h"

#include "smtk/mesh/core/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/SetProperty_xml.h"

using namespace smtk::model;
using smtk::attribute::StringItem;
using smtk::attribute::DoubleItem;
using smtk::attribute::IntItem;

namespace smtk
{
namespace model
{

template <typename V, typename VL, typename VI>
void SetPropertyValue(
  const std::string& name, typename VI::Ptr item, std::vector<smtk::model::EntityPtr>& entities)
{
  if (!item || item->numberOfValues() == 0)
  {
    // Erase the property of this type from these entities,
    // if they had the property in the first place.
    for (smtk::model::EntityPtr& entity : entities)
    {
      entity->properties().erase<VL>(name);
    }
  }
  else
  {
    // Get the array of values from the item.
    VL values;
    values.reserve(item->numberOfValues());
    for (std::size_t i = 0; i < item->numberOfValues(); ++i)
    {
      values.push_back(item->value(i));
    }

    // Add or overwrite the property with the values.
    for (smtk::model::EntityPtr& entity : entities)
    {
      entity->properties().get<VL>()[name] = values;
    }
  }
}

SetProperty::Result SetProperty::operateInternal()
{
  smtk::attribute::StringItemPtr nameItem = this->parameters()->findString("name");
  smtk::attribute::StringItemPtr stringItem = this->parameters()->findString("string value");
  smtk::attribute::DoubleItemPtr floatItem = this->parameters()->findDouble("float value");
  smtk::attribute::IntItemPtr integerItem = this->parameters()->findInt("integer value");

  auto associations = this->parameters()->associations();
  auto entities = associations->as<std::vector<smtk::model::EntityPtr> >(
    [](smtk::resource::PersistentObjectPtr obj) {
      return std::dynamic_pointer_cast<smtk::model::Entity>(obj);
    });

  if (nameItem->value(0).empty())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  smtk::model::Resource::Ptr resource =
    std::static_pointer_cast<smtk::model::Resource>(entities[0]->resource());

  SetPropertyValue<String, StringList, StringItem>(nameItem->value(0), stringItem, entities);
  SetPropertyValue<Float, FloatList, DoubleItem>(nameItem->value(0), floatItem, entities);
  SetPropertyValue<Integer, IntegerList, IntItem>(nameItem->value(0), integerItem, entities);

  // check whether there are mesh entities's properties need to be changed
  smtk::attribute::ComponentItemPtr meshItem = this->parameters()->findComponent("meshes");
  smtk::model::EntityRefs extraModifiedModels;

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // Return the list of entities that were potentially
  // modified so that remote sessions can track what records
  // need to be re-fetched.
  smtk::attribute::ComponentItemPtr modifiedItem = result->findComponent("modified");
  for (smtk::model::EntityPtr& entity : entities)
  {
    modifiedItem->appendValue(entity);

    // if a model is in the changed entities and it is a submodel, we
    // want to label its parent model to be modified too.
    if (entity->isModel() && entity->referenceAs<model::Model>().parent().isModel())
    {
      modifiedItem->appendValue(entity->referenceAs<model::Model>().parent().component());
    }
  }

  return result;
}

const char* SetProperty::xmlDescription() const
{
  return SetProperty_xml;
}

} //namespace model
} // namespace smtk
