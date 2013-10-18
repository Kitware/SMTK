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
