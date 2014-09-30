//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/view/Base.h"
using namespace smtk::view;

//----------------------------------------------------------------------------
Base::Base(const std::string &myTitle): m_title(myTitle)
{
}

//----------------------------------------------------------------------------
Base::~Base()
{
}
//----------------------------------------------------------------------------
std::string Base::type2String(Base::Type t)
{
  switch (t)
    {
    case ATTRIBUTE:
      return "Attribute";
    case GROUP:
      return "Group";
    case INSTANCED:
      return "Instanced";
    case MODEL_ENTITY:
      return "ModelEntity";
    case ROOT:
      return "Root";
    case SIMPLE_EXPRESSION:
      return "SimpleExpression";
    default:
      return "";
    }
  return "Error!";
}
//----------------------------------------------------------------------------
Base::Type Base::string2Type(const std::string &s)
{
  if (s == "Attribute")
    {
    return ATTRIBUTE;
    }
  if (s == "Group")
    {
    return GROUP;
    }
  if (s == "Instanced")
    {
    return INSTANCED;
    }
  if (s == "ModelEntity")
    {
    return MODEL_ENTITY;
    }
  if (s == "Root")
    {
    return ROOT;
    }
  if (s == "SimpleExpression")
    {
    return SIMPLE_EXPRESSION;
    }
  return NUMBER_OF_TYPES;
}
//----------------------------------------------------------------------------
