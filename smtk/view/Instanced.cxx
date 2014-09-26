//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/view/Instanced.h"
using namespace smtk::view;

//----------------------------------------------------------------------------
Instanced::Instanced(const std::string &myTitle):
  Base(myTitle)
{
}

//----------------------------------------------------------------------------
Instanced::~Instanced()
{
}
//----------------------------------------------------------------------------
Base::Type Instanced::type() const
{
  return INSTANCED;
}
