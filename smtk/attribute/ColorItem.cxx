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


#include "smtk/attribute/ColorItem.h"
#include "smtk/attribute/ColorItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include <iostream>
#include <stdio.h>

using namespace smtk::attribute; 

//----------------------------------------------------------------------------
ColorItem::ColorItem(Attribute *owningAttribute, 
                   int itemPosition): 
  Item(owningAttribute, itemPosition)
{
  this->reset();
}

//----------------------------------------------------------------------------
ColorItem::ColorItem(Item *owningItem,
                   int itemPosition,
                   int subGroupPosition): 
  Item(owningItem, itemPosition, subGroupPosition)
{
  this->reset();
}

//----------------------------------------------------------------------------
bool ColorItem::
setDefinition(smtk::ConstAttributeItemDefinitionPtr adef)
{
  // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const ColorItemDefinition *def = 
    dynamic_cast<const ColorItemDefinition *>(adef.get());
  
  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if ((def == NULL) || (!Item::setDefinition(adef)))
    {
    return false;
    }
  
  this->reset();
  return true;
}

//----------------------------------------------------------------------------
ColorItem::~ColorItem()
{
}
//----------------------------------------------------------------------------
Item::Type ColorItem::type() const
{
  const ColorItemDefinition *def = 
    static_cast<const ColorItemDefinition *>(this->definition().get());
  return def->type();
}
//----------------------------------------------------------------------------
const std::string& ColorItem::label() const
{
  const ColorItemDefinition *def = 
    static_cast<const ColorItemDefinition *>(this->definition().get());
  return def->getLabel();
}
//----------------------------------------------------------------------------
bool ColorItem::setRGB(double val[3])
{
  //First - are we allowed to change the number of values?
  const ColorItemDefinition *def =
    static_cast<const ColorItemDefinition *>(this->definition().get());
  if (!def->isValueValid(val))
    {
    return false;
    }
  
  if (def->isValueValid(val))

  for(int i=0; i<3; i++)
    {
    this->m_rgb[i] = val[i];
    }
  this->m_isSet = true;
  return true;
}
//----------------------------------------------------------------------------
bool ColorItem::setRGB(double r, double g, double b)
{
  double rgb[3] = {r, g, b};
  return this->setRGB(rgb);
}
//----------------------------------------------------------------------------
void ColorItem::getRGB(double& r, double& g, double& b)
{
  r = this->m_rgb[0];
  g = this->m_rgb[1];
  b = this->m_rgb[2];
}

//----------------------------------------------------------------------------
void ColorItem::getRGB(double val[3])
{
  this->getRGB(val[0], val[1], val[2]);
}

//----------------------------------------------------------------------------
void ColorItem::reset()
{
  const ColorItemDefinition *def
    = static_cast<const ColorItemDefinition *>(this->definition().get());
  def->getDefaultRGB(m_rgb);
  this->m_isSet = false;
}
//----------------------------------------------------------------------------
