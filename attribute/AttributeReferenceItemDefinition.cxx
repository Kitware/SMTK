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


#include "attribute/AttributeReferenceItemDefinition.h"
#include "attribute/Attribute.h"
#include "attribute/AttributeReferenceItem.h"

using namespace slctk::attribute;

//----------------------------------------------------------------------------
AttributeReferenceItemDefinition::
AttributeReferenceItemDefinition(const std::string &myName):
  ItemDefinition(myName), m_definition()
{
  this->m_useCommonLabel = false;
  this->m_numberOfValues = 0;
}

//----------------------------------------------------------------------------
AttributeReferenceItemDefinition::~AttributeReferenceItemDefinition()
{
}
//----------------------------------------------------------------------------
bool 
AttributeReferenceItemDefinition::isValueValid(slctk::AttributePtr att) const
{
  if (att == NULL)
    {
    return true;
    }
  if (this->m_definition.lock() != NULL)
    {
    return att->isA(this->m_definition.lock());
    }
  return true;
}
//----------------------------------------------------------------------------
slctk::AttributeItemPtr AttributeReferenceItemDefinition::buildItem() const
{
  return slctk::AttributeItemPtr(new AttributeReferenceItem());
}
//----------------------------------------------------------------------------
void AttributeReferenceItemDefinition::setNumberOfValues(int esize)
{
  if (esize == this->m_numberOfValues)
    {
    return;
    }
  this->m_numberOfValues = esize;
  if (!this->m_useCommonLabel)
    {
    this->m_valueLabels.resize(esize);
    }
}
//----------------------------------------------------------------------------
void AttributeReferenceItemDefinition::setValueLabel(int element, const std::string &elabel)
{
  if (this->m_numberOfValues == 0)
    {
    return;
    }
  if (this->m_valueLabels.size() != this->m_numberOfValues)
    {
    this->m_valueLabels.resize(this->m_numberOfValues);
    }
  this->m_useCommonLabel = false;
  this->m_valueLabels[element] = elabel;
}
//----------------------------------------------------------------------------
void AttributeReferenceItemDefinition::setCommonValueLabel(const std::string &elabel)
{
  if (this->m_valueLabels.size() != 1)
    {
    this->m_valueLabels.resize(1);
    }
  this->m_useCommonLabel = true;
  this->m_valueLabels[0] = elabel;
}

//----------------------------------------------------------------------------
std::string AttributeReferenceItemDefinition::valueLabel(int element) const
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
