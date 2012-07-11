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

using namespace slck::attribute; 

//----------------------------------------------------------------------------
ValueComponent::ValueComponent(ValueComponentDefinition *def):
  Component(def)
{
  // Find out how many elements this component is suppose to have
  // if the size is 0 then its unbounded
  int n = def->numberOfElements();
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
    }
}

//----------------------------------------------------------------------------
ValueComponent::~ValueComponent()
{
}
//----------------------------------------------------------------------------
bool ValueComponent::isDiscrete() const
{
  return static_cast<ValueComponentDefinition*>(this->m_definition)->
    isDiscrete();
}
//----------------------------------------------------------------------------
void ValueComponent::setDiscreteIndex(int element, int index)
{
  if (!this->isDiscrete())
    {
    return;
    }
  ValueComponentDefintion *vdef = 
    static_cast<ValueComponentDefinition*>(this->m_definition);
  if (vdef->isDiscreteIndexValid(index))
    {
    this->m_discreteIndex[element] = index;
    this->m_isSet[element] = true;
    this->updateDiscreteValue(element);
    }
}
//----------------------------------------------------------------------------
