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
