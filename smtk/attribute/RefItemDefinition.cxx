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


#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Manager.h"
#include "smtk/attribute/RefItem.h"

#include <queue>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
RefItemDefinition::
RefItemDefinition(const std::string &myName):
  ItemDefinition(myName), m_definition()
{
  this->m_useCommonLabel = false;
  this->m_numberOfRequiredValues = 0;
}

//----------------------------------------------------------------------------
RefItemDefinition::~RefItemDefinition()
{
}
//----------------------------------------------------------------------------
Item::Type RefItemDefinition::type() const
{
  return Item::ATTRIBUTE_REF;
}

//----------------------------------------------------------------------------
bool
RefItemDefinition::isValueValid(smtk::attribute::AttributePtr att) const
{
  if (!att)
    {
    return true;
    }
  if (this->m_definition.lock())
    {
    return att->isA(this->m_definition.lock());
    }
  return true;
}
//----------------------------------------------------------------------------
smtk::attribute::ItemPtr
RefItemDefinition::buildItem(Attribute *owningAttribute,
                                      int itemPosition) const
{
  return smtk::attribute::ItemPtr(new RefItem(owningAttribute,
                                                     itemPosition));
}
//----------------------------------------------------------------------------
smtk::attribute::ItemPtr
RefItemDefinition::buildItem(Item *owningItem,
                                      int itemPosition,
                                      int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new RefItem(owningItem,
                                                      itemPosition,
                                                      subGroupPosition));
}
//----------------------------------------------------------------------------
void RefItemDefinition::setNumberOfRequiredValues(std::size_t esize)
{
  if (esize == this->m_numberOfRequiredValues)
    {
    return;
    }
  this->m_numberOfRequiredValues = esize;
  if (!this->m_useCommonLabel)
    {
    this->m_valueLabels.resize(esize);
    }
}
//----------------------------------------------------------------------------
void RefItemDefinition::setValueLabel(std::size_t element, const std::string &elabel)
{
  if (this->m_numberOfRequiredValues == 0)
    {
    return;
    }
  if (this->m_valueLabels.size() != this->m_numberOfRequiredValues)
    {
    this->m_valueLabels.resize(this->m_numberOfRequiredValues);
    }
  this->m_useCommonLabel = false;
  this->m_valueLabels[element] = elabel;
}
//----------------------------------------------------------------------------
void RefItemDefinition::setCommonValueLabel(const std::string &elabel)
{
  if (this->m_valueLabels.size() != 1)
    {
    this->m_valueLabels.resize(1);
    }
  this->m_useCommonLabel = true;
  this->m_valueLabels[0] = elabel;
}

//----------------------------------------------------------------------------
std::string RefItemDefinition::valueLabel(std::size_t element) const
{
  if (this->m_useCommonLabel)
    {
    return this->m_valueLabels[0];
    }
  if (this->m_valueLabels.size())
    {
    return this->m_valueLabels[element];
    }
  return ""; // If we threw execeptions this method could return const string &
}
//----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr
smtk::attribute::RefItemDefinition::
createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  std::size_t i;

  smtk::attribute::RefItemDefinitionPtr newRef =
    smtk::attribute::RefItemDefinition::New(this->name());
  ItemDefinition::copyTo(newRef);

  // Set attributeDefinition (if possible)
  if (this->attributeDefinition())
    {
    std::string typeStr = this->attributeDefinition()->type();
    smtk::attribute::DefinitionPtr def = info.ToManager.findDefinition(typeStr);
    if (def)
      {
      newRef->setAttributeDefinition(def);
      }
    else
      {
      std::cout << "Adding definition \"" << typeStr
        << "\" to copy-definition queue"
        << std::endl;
      info.UnresolvedRefItems.push(std::make_pair(typeStr, newRef));
      }
    }

  newRef->setNumberOfRequiredValues(m_numberOfRequiredValues);

  // Labels
  if (m_useCommonLabel)
    {
    newRef->setCommonValueLabel(m_valueLabels[0]);
    }
  else if (this->hasValueLabels())
    {
    for (i=0; i<m_valueLabels.size(); ++i)
      {
      newRef->setValueLabel(i, m_valueLabels[i]);
      }
    }

  return newRef;
}
//----------------------------------------------------------------------------
