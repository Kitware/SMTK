//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME ModelEntityItem.h - Hold a model entity as an attribute value.
// .SECTION Description
// An attribute whose value is a model entity (such as an edge or face).
//
// While the actual value stored is a Cursor, only the UUID of the model entity
// is saved when the attribute is serialized.
// Then, when deserialized, the attribute system's refModelManager()
// is used to recreate the cursor.
// (Thus the UUID is assumed to be present in the model manager referenced by
// the attribute system. See smtk::attrib::System::refModelManager().)
// If the entity is not present, the returned smtk::model::Cursor
// instances will be invalid and no type checking of attribute values
// can be performed.
//
// .SECTION See Also

#ifndef __smtk_attribute_ModelEntityItem_h
#define __smtk_attribute_ModelEntityItem_h

#include "smtk/attribute/Item.h"
#include "smtk/SMTKCoreExports.h"

namespace smtk
{
  namespace common { class UUID; }
  namespace attribute
  {

class Attribute;
class ModelEntityItemDefinition;
class SMTKCORE_EXPORT ModelEntityItem : public Item
{
friend class ModelEntityItemDefinition;
public:
  smtkTypeMacro(ModelEntityItem);
  virtual ~ModelEntityItem();
  virtual Item::Type type() const;

  std::size_t numberOfValues() const;
  bool setNumberOfValues(std::size_t newSize);

  std::size_t numberOfRequiredValues() const;
  smtk::model::Cursor value(std::size_t element = 0) const;
  bool setValue(const smtk::model::Cursor& val);
  bool setValue(std::size_t element, const smtk::model::Cursor& val);
  bool appendValue(const smtk::model::Cursor& val);
  bool removeValue(std::size_t element);
  virtual void reset();
  virtual std::string valueAsString() const;
  virtual std::string valueAsString(std::size_t element) const;
  virtual bool isSet(std::size_t element = 0) const;
  virtual void unset(std::size_t element = 0);
  virtual void copyFrom(const smtk::attribute::ItemPtr sourceItem,
                        smtk::attribute::Item::CopyInfo& info);

protected:
  ModelEntityItem(Attribute *owningAttribute, int itemPosition);
  ModelEntityItem(Item *owningItem, int myPosition, int mySubGroupPosition);

  virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr def);

  smtk::model::CursorArray m_values;
};

  } // namespace attribute
} // namespace smtk

#endif /* __smtk_attribute_ModelEntityItem_h */
