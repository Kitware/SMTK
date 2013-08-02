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


#include "smtk/attribute/ColorItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ColorItem.h"
#include <cstring>
#include <sstream>
#include <iostream>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
ColorItemDefinition::
ColorItemDefinition(const std::string &myName):
  ItemDefinition(myName)
{
  this->setDefaultRGB(1.0, 1.0, 1.0);
  this->setLabel("Color");
}

//----------------------------------------------------------------------------
ColorItemDefinition::~ColorItemDefinition()
{
}

//----------------------------------------------------------------------------
Item::Type ColorItemDefinition::type() const
{
  return Item::COLOR;
}

//----------------------------------------------------------------------------
bool ColorItemDefinition::isValueValid(double val[3]) const
{
  return val[0]>=0.0 && val[0]<=1.0 &&
         val[1]>=0.0 && val[1]<=1.0 &&
         val[2]>=0.0 && val[2]<=1.0;
}
//----------------------------------------------------------------------------
smtk::AttributeItemPtr ColorItemDefinition::buildItem(Attribute *owningAttribute,
                                      int itemPosition) const
{
  return smtk::AttributeItemPtr(new ColorItem(owningAttribute,
                                              itemPosition));
}
//----------------------------------------------------------------------------
smtk::AttributeItemPtr ColorItemDefinition::buildItem(Item *owningItem,
                                                      int itemPosition,
                                                      int subGroupPosition) const
{
  return smtk::AttributeItemPtr(new ColorItem(owningItem,
                                              itemPosition,
                                              subGroupPosition));
}
//----------------------------------------------------------------------------
bool ColorItemDefinition::convertRGBFromString(std::string& dval, double rgb[3])
{
  char * pch;
  pch = strtok(const_cast<char*>(dval.c_str()), " ,");
  int i=0;
  while (pch != NULL && i<3)
    {
    std::istringstream strval(pch);
    strval >> rgb[i++];
    pch = strtok (NULL, " ,");
    }
  return i==3;
}
