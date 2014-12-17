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

#include "smtk/model/Bridge.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ModelEntity.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

using namespace smtk::model;
using smtk::attribute::StringItem;
using smtk::attribute::DoubleItem;
using smtk::attribute::IntItem;

namespace smtk {
  namespace model {

template<typename V, typename VL, typename VD, typename VI>
void SetPropertyValue(const std::string& name, typename VI::Ptr item, CursorArray& entities)
{
  CursorArray::iterator it;
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

// local helper
bool SetProperty::ableToOperate()
{
  return this->specification()->isValid();
}

smtk::model::OperatorResult SetProperty::operateInternal()
{
  smtk::attribute::StringItemPtr nameItem = this->findString("name");
  smtk::attribute::StringItemPtr stringItem = this->findString("string value");
  smtk::attribute::DoubleItemPtr floatItem = this->findDouble("float value");
  smtk::attribute::IntItemPtr integerItem = this->findInt("integer value");

  CursorArray entities = this->associatedEntitiesAs<CursorArray>();

  if (nameItem->value(0).empty())
    return this->createResult(smtk::model::OPERATION_FAILED);

  SetPropertyValue<String,StringList,StringData,StringItem>(
    nameItem->value(0), stringItem, entities);
  SetPropertyValue<Float,FloatList,FloatData,DoubleItem>(
    nameItem->value(0), floatItem, entities);
  SetPropertyValue<Integer,IntegerList,IntegerData,IntItem>(
    nameItem->value(0), integerItem, entities);

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);

  // Return the list of entities that were potentially
  // modified so that remote bridges can track what records
  // need to be re-fetched.
  smtk::attribute::ModelEntityItem::Ptr resultEntities =
    result->findModelEntity("entities");

  int numEntitiesOut = static_cast<int>(entities.size());
  resultEntities->setNumberOfValues(numEntitiesOut);
  CursorArray::iterator it = entities.begin();
  for (int i = 0; i < numEntitiesOut; ++i, ++it)
    resultEntities->setValue(i, *it);

  return result;
}

  } //namespace bridge
} // namespace smtk

#include "smtk/model/SetProperty_xml.h"

smtkImplementsModelOperator(
  smtk::model::SetProperty,
  set_property,
  "set property",
  SetProperty_xml,
  smtk::model::Bridge);
