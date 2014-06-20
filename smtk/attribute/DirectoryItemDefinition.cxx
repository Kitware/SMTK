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


#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DirectoryItem.h"

using namespace smtk::attribute;

//----------------------------------------------------------------------------
DirectoryItemDefinition::
DirectoryItemDefinition(const std::string &myName):
  ItemDefinition(myName), m_shouldExist(false), m_shouldBeRelative(false),
  m_useCommonLabel(false), m_numberOfRequiredValues(1)
{
}

//----------------------------------------------------------------------------
DirectoryItemDefinition::~DirectoryItemDefinition()
{
}
//----------------------------------------------------------------------------
Item::Type DirectoryItemDefinition::type() const
{
  return Item::DIRECTORY;
}

//----------------------------------------------------------------------------
bool 
DirectoryItemDefinition::isValueValid(const std::string &/*val*/) const
{
  return true;
}
//----------------------------------------------------------------------------
smtk::attribute::ItemPtr
DirectoryItemDefinition::buildItem(Attribute *owningAttribute,
                                   int itemPosition) const
{
  return smtk::attribute::ItemPtr(new DirectoryItem(owningAttribute,
                                                   itemPosition));
}
//----------------------------------------------------------------------------
smtk::attribute::ItemPtr
DirectoryItemDefinition::buildItem(Item *owningItem,
                                   int itemPosition,
                                   int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new DirectoryItem(owningItem,
                                                   itemPosition,
                                                   subGroupPosition));
}
//----------------------------------------------------------------------------
void DirectoryItemDefinition::setNumberOfRequiredValues(std::size_t esize)
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
void DirectoryItemDefinition::setValueLabel(std::size_t element, const std::string &elabel)
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
void DirectoryItemDefinition::setCommonValueLabel(const std::string &elabel)
{
  if (this->m_valueLabels.size() != 1)
    {
    this->m_valueLabels.resize(1);
    }
  this->m_useCommonLabel = true;
  this->m_valueLabels[0] = elabel;
}

//----------------------------------------------------------------------------
std::string DirectoryItemDefinition::valueLabel(std::size_t element) const
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
smtk::attribute::DirectoryItemDefinition::
createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  (void)info;
  smtk::attribute::DirectoryItemDefinitionPtr instance =
    smtk::attribute::DirectoryItemDefinition::New(this->name());
  ItemDefinition::copyTo(instance);

  instance->setNumberOfRequiredValues(m_numberOfRequiredValues);

  // Add label(s)
  if (m_useCommonLabel)
    {
    instance->setCommonValueLabel(m_valueLabels[0]);
    }
  else if (this->hasValueLabels())
    {
    for (std::size_t i=0; i<m_valueLabels.size(); ++i)
      {
      instance->setValueLabel(i, m_valueLabels[i]);
      }
    }

  instance->setShouldExist(m_shouldExist);
  instance->setShouldBeRelative(m_shouldBeRelative);

  return instance;
}
//----------------------------------------------------------------------------
