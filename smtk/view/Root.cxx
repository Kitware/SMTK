//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/view/Root.h"
using namespace smtk::view;

//----------------------------------------------------------------------------
Root::Root(const std::string &myTitle): Group(myTitle)
{
  this->m_defaultColor[0] = 1.0;
  this->m_defaultColor[1] = 1.0;
  this->m_defaultColor[2] = 0.5;
  this->m_defaultColor[3] = 1.0;
  this->m_invalidColor[0] = 1.0;
  this->m_invalidColor[1] = 0.5;
  this->m_invalidColor[2] = 0.5;
  this->m_invalidColor[3] = 1.0;
  this->m_advancedBold = true;
  this->m_advancedItalic = false;
  this->m_maxValueLabelLen = 200;
  this->m_minValueLabelLen = 50;
}

//----------------------------------------------------------------------------
Root::~Root()
{
}
//----------------------------------------------------------------------------
Base::Type Root::type() const
{
  return ROOT;
}
//----------------------------------------------------------------------------
void Root::setMaxValueLabelLength(int l)
{
  if(l>=this->m_minValueLabelLen)
    {
    this->m_maxValueLabelLen = l;
    }
}
//----------------------------------------------------------------------------
void Root::setMinValueLabelLength(int l)
{
  if(l<=this->m_maxValueLabelLen)
    {
    this->m_minValueLabelLen = l;
    }
}
