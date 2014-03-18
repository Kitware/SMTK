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


#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
using namespace smtk::attribute;

//----------------------------------------------------------------------------
GroupItem::GroupItem(Attribute *owningAttribute,
                     int itemPosition):
  Item(owningAttribute, itemPosition)
{
}

//----------------------------------------------------------------------------
GroupItem::GroupItem(Item *inOwningItem,
                     int itemPosition,
                     int mySubGroupPosition):
  Item(inOwningItem, itemPosition, mySubGroupPosition)
{
}

//----------------------------------------------------------------------------
GroupItem::~GroupItem()
{
  // This group is going away so make sure any items that are
  // being held externally no longer think they are owned by it
  this->detachAllItems();
 }
//----------------------------------------------------------------------------
void GroupItem::detachAllItems()
{
  // Detatch all top level items contained in this group
  std::size_t i, j, n, m;
  n = this->m_items.size();
  for (i = 0; i < n; i++)
    {
    std::vector<smtk::attribute::ItemPtr> &items = this->m_items[i];
    m = items.size();
    for (j = 0; j < m; j++)
      {
      items[j]->detachOwningItem();
      }
    }
}
//----------------------------------------------------------------------------
Item::Type GroupItem::type() const
{
  return GROUP;
}
//----------------------------------------------------------------------------
bool
GroupItem::setDefinition(smtk::attribute::ConstItemDefinitionPtr gdef)
{
   // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const GroupItemDefinition *def =
    dynamic_cast<const GroupItemDefinition *>(gdef.get());
  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if ((def == NULL) || (!Item::setDefinition(gdef)))
    {
    return false;
    }
  this->m_definition = gdef;
  std::size_t i, n = def->numberOfRequiredGroups();
  if (n)
    {
    this->m_items.resize(n);
    for (i = 0; i < n; i++)
      {
      def->buildGroup(this, static_cast<int>(i));
      }
    }
  return true;
}
//----------------------------------------------------------------------------
void GroupItem::reset()
{
  const GroupItemDefinition *def =
    dynamic_cast<const GroupItemDefinition *>(this->m_definition.get());
  std::size_t i, n = def->numberOfRequiredGroups();
  if (this->numberOfGroups() != n)
    {
    this->setNumberOfGroups(n);
    }
  if (!n)
    {
    this->detachAllItems();
    this->m_items.clear();
    }
  else
    {
    for (i = 0; i < n; i++)
      {
      std::size_t j, m = this->m_items[i].size();
      for (j = 0; j < m; j++)
        {
        this->m_items[i][j]->reset();
        }
      }
    }
  Item::reset();
}
//----------------------------------------------------------------------------
bool GroupItem::isExtensible() const
{
  const GroupItemDefinition *def =
    static_cast<const GroupItemDefinition*>(this->m_definition.get());
  if (!def)
    {
    return false;
    }
  return def->isExtensible();
}
//----------------------------------------------------------------------------
std::size_t GroupItem::numberOfItemsPerGroup() const
{
  const GroupItemDefinition *def =
    static_cast<const GroupItemDefinition *>(this->definition().get());
  return def->numberOfItemDefinitions();
}
//----------------------------------------------------------------------------
std::size_t GroupItem::numberOfRequiredGroups() const
{
  const GroupItemDefinition *def =
    static_cast<const GroupItemDefinition *>(this->definition().get());
  return def->numberOfRequiredGroups();
}
//----------------------------------------------------------------------------
std::size_t GroupItem::maxNumberOfGroups() const
{
  const GroupItemDefinition *def =
    static_cast<const GroupItemDefinition*>(this->m_definition.get());
  if (!def)
    {
    return 0;
    }
  return def->maxNumberOfGroups();
}
//----------------------------------------------------------------------------
bool GroupItem::appendGroup()
{
  if (!this->isExtensible())
    {
    return false;
    }

  const GroupItemDefinition *def =
    static_cast<const GroupItemDefinition *>(this->definition().get());
  std::size_t maxN = def->maxNumberOfGroups(), n = this->numberOfGroups();
  if (maxN && (n >= maxN))
    {
    // max number of groups reached
    return false;
    }
  this->m_items.resize(n+1);
  def->buildGroup(this, static_cast<int>(n));
  return true;
}
//----------------------------------------------------------------------------
bool GroupItem::removeGroup(std::size_t element)
{
  if (!this->isExtensible())
    {
    return false;
    }
  if (this->numberOfGroups() <= this->numberOfRequiredGroups())
    {
    return false; // min number of groups reached
    }

  std::vector<smtk::attribute::ItemPtr> &items = this->m_items[element];
  std::size_t j, m = items.size();
  for(j = 0; j < m; j++)
    {
    items[j]->detachOwningItem();
    }
  this->m_items.erase(this->m_items.begin() + element);
  return true;
}
//----------------------------------------------------------------------------
bool GroupItem::setNumberOfGroups(std::size_t newSize)
{
  // If the current size is the same just return
  if (this->numberOfGroups() == newSize)
    {
    return true;
    }

  //Next - are we allowed to change the number of values?
  if (!this->isExtensible())
    {
    return false;
    }
  // Is this size between the required number and the max?
  if (newSize < this->numberOfRequiredGroups())
    {
    return false;
    }

  std::size_t n = this->maxNumberOfGroups();
  if (n && (newSize >= n))
    {
    return false; // greater than max number
    }

  const GroupItemDefinition *def =
    static_cast<const GroupItemDefinition *>(this->definition().get());
  std::size_t i;
  n = thia->numberOfGroups();
  if (newSize < n)
    {
    // We need to detach all of the items we no longer need
    std::size_t j, m;
    for (i = newSize; i < n; i++)
      {
      std::vector<smtk::attribute::ItemPtr> &items = this->m_items[i];
      m = items.size();
      for (j = 0; j < m; j++)
        {
        items[j]->detachOwningItem();
        }
      }
    }
  else
    {
    this->m_items.resize(newSize);
    for (i = n; i < newSize; i++)
      {
      def->buildGroup(this, static_cast<int>(i));
      }
    }
  return true;
}
//----------------------------------------------------------------------------
smtk::attribute::ItemPtr GroupItem::find(std::size_t element, const std::string &inName)
{
  const GroupItemDefinition *def =
    static_cast<const GroupItemDefinition *>(this->definition().get());
  int i = def->findItemPosition(inName);
  return (i < 0) ? smtk::attribute::ItemPtr() : this->m_items[element][static_cast<std::size_t>(i)];
}
//----------------------------------------------------------------------------
smtk::attribute::ConstItemPtr GroupItem::find(std::size_t element, const std::string &inName) const
{
  const GroupItemDefinition *def =
    static_cast<const GroupItemDefinition *>(this->definition().get());
  int i = def->findItemPosition(inName);
  if (i < 0)
    {
    return smtk::attribute::ConstItemPtr();
    }
  return this->m_items[element][static_cast<std::size_t>(i)];
}
//----------------------------------------------------------------------------
