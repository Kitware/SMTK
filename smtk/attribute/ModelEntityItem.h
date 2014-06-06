/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME ModelEntityItem.h - Hold a model entity as an attribute value.
// .SECTION Description
// An attribute whose value is a model entity (such as an edge or face).
//
// While the actual value stored is a Cursor, only the UUID of the model entity
// is saved when the attribute is serialized.
// Then, when deserialized, the attribute manager's refModelManager()
// is used to recreate the cursor.
// (Thus the UUID is assumed to be present in the model manager referenced by
// the attribute manager. See smtk::attrib::Manager::refModelManager().)
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
  namespace util { class UUID; }
  namespace attribute
  {

class Attribute;
class ModelEntityItemDefinition;
class SMTKCORE_EXPORT ModelEntityItem : public Item
{
friend class ModelEntityItemDefinition;
public:
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


protected:
  ModelEntityItem(Attribute *owningAttribute, int itemPosition);
  ModelEntityItem(Item *owningItem, int myPosition, int mySubGroupPosition);

  virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr def);

  smtk::model::CursorArray m_values;
};

  } // namespace attribute
} // namespace smtk

#endif /* __smtk_attribute_ModelEntityItem_h */
