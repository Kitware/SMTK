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


#include "smtk/attribute/ItemDefinition.h"
#include <iostream>
using namespace smtk::attribute; 

//----------------------------------------------------------------------------
ItemDefinition::ItemDefinition(const std::string &myName)
{
  this->m_name = myName;
  this->m_version = 0;
  this->m_advanceLevel = 0;
  this->m_isOptional = false;
  this->m_isEnabledByDefault = false;
}

//----------------------------------------------------------------------------
ItemDefinition::~ItemDefinition()
{
  std::cout << "Item Definition " << m_name << " deleted\n";
}
//----------------------------------------------------------------------------
bool ItemDefinition::isMemberOf(const std::vector<std::string> &categories) const
{
  std::size_t i, n = categories.size();
  for (i = 0; i < n; i++)
    {
    if (this->isMemberOf(categories[i]))
      {
      return true;
      }
    }
    return false;
}
//----------------------------------------------------------------------------
void ItemDefinition::updateCategories()
{
}
//----------------------------------------------------------------------------
void ItemDefinition::addCategory(const std::string &category)
{
  this->m_categories.insert(category);
}
//----------------------------------------------------------------------------
void ItemDefinition::removeCategory(const std::string &category)
{
this->m_categories.erase(category);
}
//----------------------------------------------------------------------------

