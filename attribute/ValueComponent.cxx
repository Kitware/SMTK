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


#include "attribute/ValueComponent.h"
#include "attribute/ValueComponentDefinition.h"
#include "attribute/AttributeReferenceComponent.h"
#include "attribute/AttributeReferenceComponentDefinition.h"

using namespace slctk::attribute; 

//----------------------------------------------------------------------------
ValueComponent::ValueComponent(const ValueComponentDefinition *def):
  Component(def)
{
  // Find out how many values this component is suppose to have
  // if the size is 0 then its unbounded
  int n = def->numberOfValues();
  if (n)
    {
    if (def->hasDefault())
      {
      this->m_isSet.resize(n, true);
      }
    else
      {
      this->m_isSet.resize(n, false);
      }
    if (def->isDiscrete())
      {
      this->m_discreteIndices.resize(n, def->defaultDiscreteIndex());
      }
    if (def->allowsExpressions())
      {
      int i;
      this->m_expressions.resize(n, NULL);
      for (i = 0; i < n; i++)
        {
        this->m_expressions[i] = def->buildExpressionComponent();
        }
      }
    }
}
//----------------------------------------------------------------------------
ValueComponent::~ValueComponent()
{
  const ValueComponentDefinition *def = 
    static_cast<const ValueComponentDefinition*>(this->m_definition);
  if (def->allowsExpressions())
    {
    int i, n = def->numberOfValues();
    for (i = 0; i < n; i++)
      {
      delete this->m_expressions[i];
      }
    }

}
//----------------------------------------------------------------------------
Attribute *ValueComponent::expression(int element) const
{
  const ValueComponentDefinition *def = 
    static_cast<const ValueComponentDefinition*>(this->m_definition);
  if (def->allowsExpressions())
    {
    return this->m_expressions[element]->value();
    }
  return NULL;
}
//----------------------------------------------------------------------------
bool ValueComponent::setExpression(int element, Attribute *exp)
{
  const ValueComponentDefinition *def = 
    static_cast<const ValueComponentDefinition*>(this->m_definition);
  if (def->allowsExpressions())
    {
    if (exp == NULL)
      {
      if (this->m_expressions[element]->value() != NULL)
        {
        this->m_isSet[element] = false;
        this->m_expressions[element]->setValue(NULL);
        }
      return true;
      }
    if (def->isValidExpression(exp))
      {
      this->m_isSet[element] = true;
      this->m_expressions[element]->setValue(exp);
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
bool ValueComponent::appendExpression(Attribute *exp)
{
  const ValueComponentDefinition *def = 
    static_cast<const ValueComponentDefinition*>(this->m_definition);
  if (!def->allowsExpressions())
    {
    return false;
    }
  int n = def->numberOfValues();
  if (n)
    {
    return false; // The number of values is fixed
    }
  if (!def->isValidExpression(exp))
    {
    return false; // Attribute is of the proper type
    }
  n = m_expressions.size();
  this->m_expressions.push_back(def->buildExpressionComponent());
  this->m_expressions[n]->setValue(exp);
  this->m_isSet.push_back(true);
  return true;
}
//----------------------------------------------------------------------------
bool ValueComponent::isDiscrete() const
{
  return static_cast<const ValueComponentDefinition*>(this->m_definition)->
    isDiscrete();
}
//----------------------------------------------------------------------------
void ValueComponent::setDiscreteIndex(int element, int index)
{
  if (!this->isDiscrete())
    {
    return;
    }
  const ValueComponentDefinition *def = 
    static_cast<const ValueComponentDefinition*>(this->m_definition);
  if (def->isDiscreteIndexValid(index))
    {
    this->m_discreteIndices[element] = index;
    if (def->allowsExpressions())
      {
      this->m_expressions[element]->setValue(NULL);
      }
    this->m_isSet[element] = true;
    this->updateDiscreteValue(element);
    }
}
//----------------------------------------------------------------------------
