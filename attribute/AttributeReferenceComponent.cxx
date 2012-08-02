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


#include "attribute/AttributeReferenceComponent.h"
#include "attribute/AttributeReferenceComponentDefinition.h"
#include "attribute/Attribute.h"
#include <iostream>

using namespace slctk::attribute; 

//----------------------------------------------------------------------------
AttributeReferenceComponent::
AttributeReferenceComponent(const AttributeReferenceComponentDefinition *def):
  ValueComponent(def)
{
  int n = def->numberOfValues();
  if (n)
    {
    this->m_values.resize(n, NULL);
    }
}

//----------------------------------------------------------------------------
AttributeReferenceComponent::~AttributeReferenceComponent()
{
  const AttributeReferenceComponentDefinition *def = 
    static_cast<const AttributeReferenceComponentDefinition *>(this->definition());
  int i, n = def->numberOfValues();
  for (i = 0; i < n; i++)
    {
    if (this->m_values[i] != NULL)
      {
      this->m_values[i]->unregisterComponent(this);
      }
    }
}
//----------------------------------------------------------------------------
Component::Type AttributeReferenceComponent::type() const
{
  return ATTRIBUTE_REFERENCE;
}

//----------------------------------------------------------------------------
bool AttributeReferenceComponent::setValue(int element, Attribute *att)
{
  const AttributeReferenceComponentDefinition *def = 
    static_cast<const AttributeReferenceComponentDefinition *>(this->definition());
  if (def->isValueValid(att))
    {
    if (this->m_values[element])
      {
      this->m_values[element]->unregisterComponent(this);
      }
    this->m_values[element] = att;
    if (this->m_values[element])
      {
      this->m_values[element]->registerComponent(this);
      }
    else
      {
      this->m_isSet[element] = false;
      }
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void AttributeReferenceComponent::updateDiscreteValue(int)
{
  std::cerr << "AttributeReferenceComponent::updateDiscreteValue - Not Implemented!\n";
}
//----------------------------------------------------------------------------
const std::string &
AttributeReferenceComponent::valueAsString(int element, 
                                      const std::string &format) const
{
  // For the initial design we will use sprintf and force a limit of 300 char
  char dummy[300];
  sprintf(dummy, format.c_str(), this->m_values[element]->id());
  this->m_tempString = dummy;
  return this->m_tempString;
}
//----------------------------------------------------------------------------
bool
AttributeReferenceComponent::appendValue(Attribute *val)
{
  //First - are we allowed to change the number of values?
  const AttributeReferenceComponentDefinition *def =
    static_cast<const AttributeReferenceComponentDefinition *>(this->definition());
  int n = def->numberOfValues();
  if (n)
    {
    return false; // The number of values is fixed
    }
  
  if (def->isValueValid(val))
    {
    this->m_values.push_back(val);
    if (val != NULL)
      {
      this->m_isSet.push_back(true);
      val->registerComponent(this);
      }
    else
      {
      this->m_isSet.push_back(false);
      }
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool
AttributeReferenceComponent::removeValue(int element)
{
  //First - are we allowed to change the number of values?
  const AttributeReferenceComponentDefinition *def =
    static_cast<const AttributeReferenceComponentDefinition *>(this->definition());
  int n = def->numberOfValues();
  if (n)
    {
    return false; // The number of values is fixed
    }
  if (this->m_values[element] != NULL)
    {
    this->m_values[element]->unregisterComponent(this);
    }
  this->m_values.erase(this->m_values.begin()+element);
  this->m_isSet.erase(this->m_isSet.begin()+element);
  if (def->isDiscrete())
    {
    this->m_discreteIndices.erase(this->m_discreteIndices.begin()+element);
    }
  return true;
}
//----------------------------------------------------------------------------
bool
AttributeReferenceComponent::setToDefault(int)
{
  std::cerr << "AttributeReferenceComponent::setToDefault - Not Implemented!\n";
  return false;
}
//----------------------------------------------------------------------------
void
AttributeReferenceComponent::reset()
{
  const AttributeReferenceComponentDefinition *def
    = static_cast<const AttributeReferenceComponentDefinition *>(this->definition());
  // Was the initial size 0?
  int i, n = def->numberOfValues();
  if (!n)
    {
    this->m_values.clear();
    this->m_isSet.clear();
    this->m_discreteIndices.clear();
    return;
    }
  for (i = 0; i < n; i++)
    {
    this->unset(i);
    }
}
//----------------------------------------------------------------------------
