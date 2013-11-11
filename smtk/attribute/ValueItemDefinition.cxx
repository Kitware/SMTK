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


#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/RefItemDefinition.h"

using namespace smtk::attribute; 

//----------------------------------------------------------------------------
ValueItemDefinition::ValueItemDefinition(const std::string &myName):
  ItemDefinition(myName)
{
  this->m_defaultDiscreteIndex = -1;
  this->m_hasDefault = false;
  this->m_useCommonLabel = false;
  this->m_numberOfRequiredValues = 1;
  this->m_expressionDefinition = RefItemDefinition::New("expression");
  this->m_expressionDefinition->setNumberOfRequiredValues(1);
}

//----------------------------------------------------------------------------
ValueItemDefinition::~ValueItemDefinition()
{
}
//----------------------------------------------------------------------------
void ValueItemDefinition::setNumberOfRequiredValues(int esize)
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
void ValueItemDefinition::setValueLabel(int element, const std::string &elabel)
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
void ValueItemDefinition::setCommonValueLabel(const std::string &elabel)
{
  if (this->m_valueLabels.size() != 1)
    {
    this->m_valueLabels.resize(1);
    }
  this->m_useCommonLabel = true;
  this->m_valueLabels[0] = elabel;
}

//----------------------------------------------------------------------------
std::string ValueItemDefinition::valueLabel(int element) const
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
bool ValueItemDefinition::isValidExpression(smtk::attribute::AttributePtr exp) const
{
  if (this->m_expressionDefinition->attributeDefinition() &&
      this->m_expressionDefinition->isValueValid(exp))
    {
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool ValueItemDefinition::allowsExpressions() const
{
  return this->m_expressionDefinition->attributeDefinition();
}
//----------------------------------------------------------------------------
smtk::attribute::DefinitionPtr ValueItemDefinition::expressionDefinition() const
{
  return this->m_expressionDefinition->attributeDefinition();
}
//----------------------------------------------------------------------------
void
ValueItemDefinition::setExpressionDefinition(smtk::attribute::DefinitionPtr exp)
{
  this->m_expressionDefinition->setAttributeDefinition(exp);
}
//----------------------------------------------------------------------------
void 
ValueItemDefinition::buildExpressionItem(ValueItem *vitem, int position) const
{
  smtk::attribute::RefItemPtr aref =
    smtk::dynamic_pointer_cast<smtk::attribute::RefItem>
    (this->m_expressionDefinition->buildItem(vitem, position, -1));
  aref->setDefinition(this->m_expressionDefinition);
  vitem->m_expressions[position] = aref;
}
//----------------------------------------------------------------------------
void ValueItemDefinition::setDefaultDiscreteIndex(int discreteIndex)
{
  this->m_defaultDiscreteIndex = discreteIndex;
  this->m_hasDefault = true;
}
//----------------------------------------------------------------------------
