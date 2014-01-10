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
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.s
=========================================================================*/


#include "smtk/model/GroupItem.h"
using namespace smtk::model;

//----------------------------------------------------------------------------
GroupItem::GroupItem(Model *inModel, int myid, MaskType mask):
  Item(inModel, myid), m_entityMask(mask)
{
}

//----------------------------------------------------------------------------
GroupItem::~GroupItem()
{
}
//----------------------------------------------------------------------------
Item::Type GroupItem::type() const
{
  return GROUP;
}

//----------------------------------------------------------------------------
smtk::model::ItemPtr GroupItem::item(int inId) const
{
  std::map<int, smtk::model::ItemPtr>::const_iterator it =
    this->m_items.find(inId);
  if (it == this->m_items.end())
    {
    return smtk::model::ItemPtr();
    }
  return it->second;
}

//----------------------------------------------------------------------------
bool GroupItem::insert(smtk::model::ItemPtr ptr)
{
  if(!ptr)
    {
    return false;
    }
  if(this->canContain(ptr))
    {
    this->m_items[ptr->id()] = ptr;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool GroupItem::remove(smtk::model::ItemPtr ptr)
{
  if(!ptr)
    {
    return false;
    }
  int mid = ptr->id();
  std::map<int, smtk::model::ItemPtr>::iterator it = this->m_items.find(mid);
  if(it != m_items.end() && ptr == it->second)
    {
    this->m_items.erase(it);
    return true;
    }
  return false;
}
