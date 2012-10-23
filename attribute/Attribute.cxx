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


#include "attribute/Attribute.h"
#include "attribute/AttributeRefItem.h"
#include "attribute/Item.h"
#include "attribute/Definition.h"
#include "attribute/Manager.h"
#include <iostream>
using namespace slctk::attribute; 
//----------------------------------------------------------------------------
Attribute::Attribute(const std::string &myName, 
                     slctk::AttributeDefinitionPtr myDefinition, 
                     unsigned long myId):
  m_name(myName), m_definition(myDefinition), m_id(myId), m_aboutToBeDeleted(false)
{
  this->m_definition->buildAttribute(this);
}

//----------------------------------------------------------------------------
Attribute::~Attribute()
{
  this->m_aboutToBeDeleted = true;
  std::cout << "Deleting Attribute " << this->name() << "\n";
  // Clear all references to the attribute
  std::map<slctk::attribute::AttributeRefItem *, std::set<int> >::iterator it;
  for (it = this->m_references.begin(); it != this->m_references.end(); it++)
    {
    std::set<int>::iterator sit;
    for (sit = it->second.begin(); sit != it->second.end(); sit++)
      {
      it->first->unset(*sit);
      }
    }
  this->removeAllItems();
  this->removeAllAssociations();
 }
//----------------------------------------------------------------------------
void Attribute::removeAllItems()
{
  // we need to detatch all items owned bu this attribute
  std::size_t i, n = this->m_items.size();
  for (i = 0; i < n; i++)
    {
    this->m_items[i]->detachOwningAttribute();
    }
  this->m_items.clear();
}
//----------------------------------------------------------------------------
void Attribute::references(std::vector<slctk::AttributeItemPtr> &list) const
{
  list.clear();
  std::map<slctk::attribute::AttributeRefItem *, std::set<int> >::const_iterator it;
  for (it = this->m_references.begin(); it != this->m_references.end(); it++)
    {
    if (it->second.size())
      {
      list.push_back(it->first->pointer());
      }
    }
}
//----------------------------------------------------------------------------
const std::string &Attribute::type() const
{
  return this->m_definition->type();
}
//----------------------------------------------------------------------------
std::vector<std::string> Attribute::types() const
{
  std::vector<std::string> tvec;
  slctk::AttributeDefinitionPtr def = this->m_definition;
  while (def != NULL)
    {
    tvec.push_back(def->type());
    def = def->baseDefinition();
    }
  return tvec;
}
//----------------------------------------------------------------------------
bool Attribute::isA(slctk::AttributeDefinitionPtr def) const
{
  return this->m_definition->isA(def);
}
///----------------------------------------------------------------------------
bool Attribute::isMemberOf(const std::string &category) const
{
  return this->m_definition->isMemberOf(category);
}
//----------------------------------------------------------------------------
bool Attribute::isMemberOf(const std::vector<std::string> &categories) const
{
  return this->m_definition->isMemberOf(categories);
}
//----------------------------------------------------------------------------
Manager *Attribute::manager() const
{
  return this->m_definition->manager();
}
//----------------------------------------------------------------------------
slctk::AttributePtr Attribute::pointer() const
{
  Manager *m = this->manager();
  if (m)
    {
    return m->findAttribute(this->m_name);
    }
  return slctk::AttributePtr();
}
//----------------------------------------------------------------------------
void Attribute::associateEntity(slctk::ModelEntity *entity)
{
  if (this->isEntityAssociated(entity))
    {
    // Nothing to be done
    return;
    }
  this->m_entities.insert(entity);
  //TODO Need to attach attribute to the entity!
}
//----------------------------------------------------------------------------
void Attribute::disassociateEntity(slctk::ModelEntity *entity)
{
  if (this->m_entities.erase(entity))
    {
    // TODO Need to detatch the attribute from the entity
    }
}
//----------------------------------------------------------------------------
void Attribute::removeAllAssociations()
{
  std::set<slctk::ModelEntity *>::iterator it;
  for (it = this->m_entities.begin(); it != this->m_entities.end(); it++)
    {
    // TODO Need to detatch the attribute from the entity
    }
}
//----------------------------------------------------------------------------
slctk::ConstAttributeItemPtr Attribute::find(const std::string &name) const
{
  int i = this->m_definition->findItemPosition(name);
  if (i < 0)
    {
    return slctk::ConstAttributeItemPtr();
    }
  return this->m_items[i];
}

//----------------------------------------------------------------------------
slctk::AttributeItemPtr Attribute::find(const std::string &name)
{
  int i = this->m_definition->findItemPosition(name);
  return (i < 0) ? slctk::AttributeItemPtr() : this->m_items[i];
}
//----------------------------------------------------------------------------
