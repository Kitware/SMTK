//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/view/Group.h"
using namespace smtk::view;

//----------------------------------------------------------------------------
Group::Group(const std::string &myTitle): Base(myTitle)
{
  this->m_style = Group::TABBED;
}

//----------------------------------------------------------------------------
Group::~Group()
{
}
//----------------------------------------------------------------------------
Base::Type Group::type() const
{
  return GROUP;
}
