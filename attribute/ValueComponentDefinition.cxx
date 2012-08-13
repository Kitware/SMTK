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


#include "attribute/ValueComponentDefinition.h"
#include "attribute/AttributeReferenceComponent.h"
#include "attribute/AttributeReferenceComponentDefinition.h"

using namespace slctk::attribute; 

//----------------------------------------------------------------------------
ValueComponentDefinition::ValueComponentDefinition(const std::string &myName, 
                                                   unsigned long myId):
  ComponentDefinition(myName, myId)
{
  this->m_defaultDiscreteIndex = -1;
  this->m_hasDefault = false;
  this->m_useCommonLabel = false;
  this->m_numberOfValues = 0;
  this->m_expressionDefinition = 
    slctk::AttributeReferenceComponentDefinitionPtr(new AttributeReferenceComponentDefinition("expression", 0));
  this->m_expressionDefinition->setNumberOfValues(1);
}

//----------------------------------------------------------------------------
ValueComponentDefinition::~ValueComponentDefinition()
{
}
//----------------------------------------------------------------------------
void ValueComponentDefinition::setNumberOfValues(int esize)
{
  if (esize == this->m_numberOfValues)
    {
    return;
    }
  this->m_numberOfValues = esize;
  if (this->m_hasDefault && (!this->m_useCommonLabel))
    {
    this->m_valueLables.resize(esize);
    }
}
//----------------------------------------------------------------------------
void ValueComponentDefinition::setValueLabel(int element, const std::string &elabel)
{
  if (this->m_numberOfValues == 0)
    {
    return;
    }
  if (this->m_valueLables.size() != this->m_numberOfValues)
    {
    this->m_valueLables.resize(this->m_numberOfValues);
    }
  this->m_useCommonLabel = false;
  this->m_valueLables[element] = elabel;
}
//----------------------------------------------------------------------------
void ValueComponentDefinition::setCommonValueLable(const std::string &elable)
{
  if (this->m_valueLables.size() != 1)
    {
    this->m_valueLables.resize(1);
    }
  this->m_useCommonLabel = true;
  this->m_valueLables[0] = elable;
}

//----------------------------------------------------------------------------
std::string ValueComponentDefinition::valueLable(int element) const
{
  if (this->m_useCommonLabel)
    {
    return this->m_valueLables[0];
    }
  if (this->m_valueLables.size())
    {
    return this->m_valueLables[element];
    }
  return ""; // If we threw execeptions this method could return const string &
}
//----------------------------------------------------------------------------
bool ValueComponentDefinition::isValidExpression(slctk::AttributePtr exp) const
{
  if ((this->m_expressionDefinition->attributeDefinition() != NULL) && 
      this->m_expressionDefinition->isValueValid(exp))
    {
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool ValueComponentDefinition::allowsExpressions() const
{
  return this->m_expressionDefinition->attributeDefinition() != NULL;
}
//----------------------------------------------------------------------------
slctk::AttributeDefinitionPtr ValueComponentDefinition::expressionDefinition() const
{
  return this->m_expressionDefinition->attributeDefinition();
}
//----------------------------------------------------------------------------
void
ValueComponentDefinition::setExpressionDefinition(slctk::AttributeDefinitionPtr exp)
{
  this->m_expressionDefinition->setAttributeDefinition(exp);
}
//----------------------------------------------------------------------------
AttributeReferenceComponent *
ValueComponentDefinition::buildExpressionComponent() const
{
  return static_cast<AttributeReferenceComponent *>
    (this->m_expressionDefinition->buildComponent());
}
//----------------------------------------------------------------------------
