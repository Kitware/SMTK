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


#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include <iostream>
#include <stdio.h>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
RefItem::RefItem(Attribute *owningAttribute,
                                   int itemPosition):
  Item(owningAttribute, itemPosition)
{
}

//----------------------------------------------------------------------------
RefItem::RefItem(Item *owningItem,
                                   int itemPosition,
                                   int mySubGroupPosition):
  Item(owningItem, itemPosition, mySubGroupPosition)
{
}

//----------------------------------------------------------------------------
bool RefItem::
setDefinition(smtk::ConstAttributeItemDefinitionPtr adef)
{
  // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const RefItemDefinition *def =
    dynamic_cast<const RefItemDefinition *>(adef.get());

  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if ((def == NULL) || (!Item::setDefinition(adef)))
    {
    return false;
    }
  int n = def->numberOfRequiredValues();
  if (n)
    {
    this->m_values.resize(n);
    }
  return true;
}

//----------------------------------------------------------------------------
RefItem::~RefItem()
{
  this->clearAllReferences();
}
//----------------------------------------------------------------------------
void RefItem::clearAllReferences()
{
  std::size_t i, n = this->m_values.size();
  Attribute *att;
  for (i = 0; i < n; i++)
    {
    att = this->m_values[i].lock().get();
    if (att)
      {
      att->removeReference(this);
      }
    }
}
//----------------------------------------------------------------------------
Item::Type RefItem::type() const
{
  return ATTRIBUTE_REF;
}

//----------------------------------------------------------------------------
int RefItem::numberOfRequiredValues() const
{
  const RefItemDefinition *def =
    static_cast<const RefItemDefinition*>(this->m_definition.get());
  if (def == NULL)
    {
    return 0;
    }
  return def->numberOfRequiredValues();
}
//----------------------------------------------------------------------------
bool RefItem::setValue(int element, smtk::AttributePtr att)
{
  const RefItemDefinition *def =
    static_cast<const RefItemDefinition *>(this->definition().get());
  if (def->isValueValid(att))
    {
    Attribute *attPtr = this->m_values[element].lock().get();
    if (attPtr != NULL)
      {
      attPtr->removeReference(this, element);
      }
    this->m_values[element] = att;
    att->addReference(this, element);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
std::string
RefItem::valueAsString(int element,
                                      const std::string &format) const
{
  // For the initial design we will use sprintf and force a limit of 300 char
  char dummy[300];
  if (format != "")
    {
    sprintf(dummy, format.c_str(), this->m_values[element].lock()->name().c_str());
    }
  else
    {
    sprintf(dummy, "%s", this->m_values[element].lock()->name().c_str());
    }
  return dummy;
}
//----------------------------------------------------------------------------
bool
RefItem::appendValue(smtk::AttributePtr val)
{
  //First - are we allowed to change the number of values?
  const RefItemDefinition *def =
    static_cast<const RefItemDefinition *>(this->definition().get());
  int n = def->numberOfRequiredValues();
  if (n)
    {
    return false; // The number of values is fixed
    }

  if (def->isValueValid(val))
    {
    this->m_values.push_back(val);
    val->addReference(this, this->m_values.size() - 1);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool
RefItem::removeValue(int element)
{
  //First - are we allowed to change the number of values?
  const RefItemDefinition *def =
    static_cast<const RefItemDefinition *>(this->definition().get());
  int n = def->numberOfRequiredValues();
  if (n)
    {
    return false; // The number of values is fixed
    }
  // Tell the attribute we are no longer referencing it (if needed)
  Attribute *att = this->m_values[element].lock().get();
  if (att != NULL)
    {
    att->removeReference(this, element);
    }
  this->m_values.erase(this->m_values.begin()+element);
  return true;
}
//----------------------------------------------------------------------------
bool
RefItem::setNumberOfValues(std::size_t newSize)
{
  // If the current size is the same just return
  if (this->numberOfValues() == newSize)
    {
    return true;
    }

  //Next - are we allowed to change the number of values?
  const RefItemDefinition *def =
    static_cast<const RefItemDefinition *>(this->definition().get());
  std::size_t n = def->numberOfRequiredValues();
  if (n)
    {
    return false; // The number of values is fixed
    }
  if (newSize < n)
    {
    std::size_t i;
    Attribute *att;
    for (i = newSize; i < n; i++)
      {
      att = this->m_values[i].lock().get();
      if (att != NULL)
        {
        att->removeReference(this, i);
        }
      }
    }
  this->m_values.resize(newSize);
  return true;
}
//----------------------------------------------------------------------------
void
RefItem::unset(int element)
{
  Attribute *att = this->m_values[element].lock().get();
  if (att == NULL)
    {
    return;
    }
  this->m_values[element].reset();
  // See if we need to tell the attribute we are no longer referencing it
  if (!att->isAboutToBeDeleted())
    {
    att->removeReference(this, element);
    }
}
//----------------------------------------------------------------------------
void
RefItem::reset()
{
  const RefItemDefinition *def
    = static_cast<const RefItemDefinition *>(this->definition().get());
  // Was the initial size 0?
  int i, n = def->numberOfRequiredValues();
  if (!n)
    {
    this->clearAllReferences();
    this->m_values.clear();
    return;
    }
  for (i = 0; i < n; i++)
    {
    this->unset(i);
    }
  Item::reset();
}
//----------------------------------------------------------------------------
