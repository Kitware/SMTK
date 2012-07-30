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


#include "attribute/AttributeReferenceComponentDefinition.h"
#include "attribute/Attribute.h"
#include "attribute/AttributeReferenceComponent.h"

using namespace slctk::attribute;

//----------------------------------------------------------------------------
AttributeReferenceComponentDefinition::
AttributeReferenceComponentDefinition(const std::string &myName,
                                      unsigned long myId):
  ValueComponentDefinition(myName, myId), m_definition(NULL)
{
}

//----------------------------------------------------------------------------
AttributeReferenceComponentDefinition::~AttributeReferenceComponentDefinition()
{
}
//----------------------------------------------------------------------------
bool AttributeReferenceComponentDefinition::hasRange() const
{
  return false;
}
//----------------------------------------------------------------------------
bool AttributeReferenceComponentDefinition::isValueValid(Attribute *att) const
{
  if (att == NULL)
    {
    return true;
    }
  if (this->m_definition != NULL)
    {
    return att->isA(this->m_definition);
    }
  return true;
}
//----------------------------------------------------------------------------
Component *AttributeReferenceComponentDefinition::createComponent()
{
  return new AttributeReferenceComponent(this);
}
//----------------------------------------------------------------------------
