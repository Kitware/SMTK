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


#include "attribute/Item.h"
#include "attribute/ItemDefinition.h"
#include <iostream>
using namespace slctk::attribute; 

//----------------------------------------------------------------------------
Item::Item():
  m_isEnabled(true), m_definition()
{
}

//----------------------------------------------------------------------------
Item::~Item()
{
  std::cout << "Item " << this->name() << " deleted\n";
}
//----------------------------------------------------------------------------
std::string Item::name() const
{
  if (this->m_definition == NULL)
    {
    return "";
    }
  return this->m_definition->name();
}
//----------------------------------------------------------------------------
bool Item::setDefinition(slctk::ConstAttributeItemDefinitionPtr def)
{
  if (this->m_definition != NULL)
    {
    return false;
    }
  this->m_definition = def;
  return true;
}
//----------------------------------------------------------------------------
bool Item::isOptional() const
{
  return this->m_definition->isOptional();
}
//----------------------------------------------------------------------------=
bool Item::isEnabled() const
{
  return this->isOptional() ? this->m_isEnabled : true;
}
//----------------------------------------------------------------------------
bool Item::isMemberOf(const std::string &catagory) const
{
  return this->definition()->isMemberOf(catagory);
}
//----------------------------------------------------------------------------
bool Item::isMemberOf(const std::vector<std::string> &catagories) const
{
  return this->definition()->isMemberOf(catagories);
}
//----------------------------------------------------------------------------
std::string Item::type2String(Item::Type t)
{
  switch (t)
    {
    case ATTRIBUTE_REF:
      return "AttributeRef";
    case DOUBLE:
      return "Double";
    case GROUP:
      return "Group";
    case INT:
      return "Int";
    case STRING:
      return "String";
    case VOID:
      return "Void";
    default:
      return "";
    }
  return "Error!";
}
//----------------------------------------------------------------------------
Item::Type Item::string2Type(const std::string &s)
{
  if (s == "AttributeRef")
    {
    return ATTRIBUTE_REF;
    }
  if (s == "Double")
    {
    return DOUBLE;
    }
  if (s == "Group")
    {
    return GROUP;
    }
  if (s == "Int")
    {
    return INT;
    }
  if (s == "String")
    {
    return STRING;
    }
  if (s == "Void")
    {
    return VOID;
    }
  return NUMBER_OF_TYPES;
}
//----------------------------------------------------------------------------
