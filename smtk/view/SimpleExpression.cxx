//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/view/SimpleExpression.h"
using namespace smtk::view;

//----------------------------------------------------------------------------
SimpleExpression::SimpleExpression(const std::string &myTitle):
  Base(myTitle)
{
}

//----------------------------------------------------------------------------
SimpleExpression::~SimpleExpression()
{
}
//----------------------------------------------------------------------------
Base::Type SimpleExpression::type() const
{
  return SIMPLE_EXPRESSION;
}
