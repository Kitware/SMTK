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


#include "attribute/ComponentDefinition.h"
using namespace slck::attribute; 

//----------------------------------------------------------------------------
ComponentDefinition::ComponentDefinition(const std::string &myName, 
                                         unsigned long myId)
{
  this->m_id = myId;
  this->m_name = myName;
  this->version = 0;
  this->m_advanceLevel = 0;
  this->m_isOptional = false;
}

//----------------------------------------------------------------------------
ComponentDefinition::~ComponentDefinition()
{
}
//----------------------------------------------------------------------------
bool ComponentDefinition::isMemberOf(const std::vector<std::string> &catagories) const
{
  size_type i, n = this->m_catagories.size();
  for (i = 0; i < n; i++)
    {
    if (this->isMemberOf(this->m_catagories[i]))
      return true;
    }
  return false;
}
//----------------------------------------------------------------------------
