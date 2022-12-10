//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/operators/SetProperty.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Entity.h"
#include "smtk/model/Model.h"

#include "smtk/operation/operators/SetProperty_xml.h"

using namespace smtk::model;
using smtk::attribute::DoubleItem;
using smtk::attribute::IntItem;
using smtk::attribute::StringItem;

namespace smtk
{
namespace operation
{

template<typename V, typename VL, typename VI>
void SetPropertyValue(
  const std::string& name,
  typename VI::Ptr item,
  smtk::attribute::ReferenceItemPtr& entities)
{
  if (!item || item->numberOfValues() == 0)
  {
    // Erase the property of this type from these entities,
    // if they had the property in the first place.
    for (smtk::resource::PersistentObjectPtr entity : *entities)
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
    for (smtk::resource::PersistentObjectPtr entity : *entities)
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

  if (nameItem->value(0).empty())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  SetPropertyValue<String, StringList, StringItem>(nameItem->value(0), stringItem, associations);
  SetPropertyValue<Float, FloatList, DoubleItem>(nameItem->value(0), floatItem, associations);
  SetPropertyValue<Integer, IntegerList, IntItem>(nameItem->value(0), integerItem, associations);

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  smtk::attribute::ComponentItemPtr modifiedItem = result->findComponent("modified");
  for (smtk::resource::PersistentObjectPtr association : *associations)
  {
    if (auto component = std::dynamic_pointer_cast<smtk::resource::Component>(association))
    {
      modifiedItem->appendValue(component);

      auto entity = std::dynamic_pointer_cast<smtk::model::Entity>(component);
      // if a model is in the changed entities and it is a submodel, we
      // want to label its parent model to be modified too.
      if (entity && entity->isModel() && entity->referenceAs<model::Model>().parent().isModel())
      {
        modifiedItem->appendValue(entity->referenceAs<model::Model>().parent().component());
      }
    }
  }

  return result;
}

const char* SetProperty::xmlDescription() const
{
  return SetProperty_xml;
}
} // namespace operation
} // namespace smtk
