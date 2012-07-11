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
using namespace slck::attribute; 

//----------------------------------------------------------------------------
ValueComponentDefinition::ValueComponentDefinition(const std::string &myName, 
                                                   unsigned long myId):
  ComponentDefinition(myName, myId)
{
  this->m_defaultDiscreteIndex = -1;
  this->m_hasDefault = false;
  this->useCommonLabel = false;
  this->m_numberOfElements = 0;
}

//----------------------------------------------------------------------------
ValueComponentDefinition::~ValueComponentDefinition()
{
}
//----------------------------------------------------------------------------
void ValueComponentDefinition::setNumberOfValues(int esize) const
{
  if (esize == this->m_numberOfElements)
    {
    return;
    }
  this->m_numberOfElements = esize;
  if (this->m_hasDefault && (!this->useCommonLabel))
    {
    this->m_elementLables.resize(esize);
    }
}
//----------------------------------------------------------------------------
void ValueComponentDefinition::setValueLabel(int element, const std::string &elabel)
{
  if (this->m_numberOfElements == 0)
    {
    return;
    }
  if (this->m_elementLables.size() != this->m_numberOfElements)
    {
    this->m_elementLables.resize(this->m_numberOfElements);
    }
  this->m_useCommonLabel = false;
  this->m_elementLables[element] = elabel;
}
//----------------------------------------------------------------------------
void ValueComponentDefinition::setCommonValueLable(const std::string &elable)
{
  if (this->m_elementLables.size() != 1)
    {
    this->m_elementLables.resize(1);
    }
  this->m_useCommonLabel = true;
  this->m_elementLables[0] = elabel;
}

//----------------------------------------------------------------------------
const std::string &ValueComponentDefinition::valueLable(int element) const
{
  if (this->m_useCommonLabel)
    {
    return this->m_elementLables[0];
    }
  if (this->m_elementLables.size())
    {
    return this->m_elementLables[element];
    }
}
//----------------------------------------------------------------------------
