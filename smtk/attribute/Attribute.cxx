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


#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Manager.h"
#include "smtk/model/Storage.h"
#include "smtk/model/Cursor.h"
#include "smtk/model/Item.h"
#include <iostream>
using namespace smtk::attribute; 
//----------------------------------------------------------------------------
Attribute::Attribute(const std::string &myName, 
                     smtk::attribute::DefinitionPtr myDefinition,
                     unsigned long myId):
  m_name(myName), m_id(myId), m_definition(myDefinition),
  m_appliesToBoundaryNodes(false), m_appliesToInteriorNodes(false),
  m_isColorSet(false), m_aboutToBeDeleted(false)
{
  this->m_definition->buildAttribute(this);
}

//----------------------------------------------------------------------------
Attribute::~Attribute()
{
  this->m_aboutToBeDeleted = true;
  // Clear all references to the attribute
  std::map<smtk::attribute::RefItem *, std::set<int> >::iterator it;
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
void Attribute::references(std::vector<smtk::attribute::ItemPtr> &list) const
{
  list.clear();
  std::map<smtk::attribute::RefItem *, std::set<int> >::const_iterator it;
  for (it = this->m_references.begin(); it != this->m_references.end(); it++)
    {
    if (it->second.size())
      {
      list.push_back(it->first->pointer());
      }
    }
}
//----------------------------------------------------------------------------
const double *Attribute::color() const
{
  if (this->m_isColorSet)
    {
    return this->m_color;
    }
  return this->m_definition->defaultColor();
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
  smtk::attribute::DefinitionPtr def = this->m_definition;
  while (def)
    {
    tvec.push_back(def->type());
    def = def->baseDefinition();
    }
  return tvec;
}
//----------------------------------------------------------------------------
bool Attribute::isA(smtk::attribute::DefinitionPtr def) const
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
/**\brief Return the model Storage instance whose entities may have attributes.
  *
  * This returns a shared pointer to smtk::model::Storage, which may be
  * null if no storage is referenced by the attribute manager (or if the
  * attribute definition does not reference a valid manager).
  */
smtk::model::StoragePtr Attribute::modelStorage() const
{
  smtk::model::StoragePtr result;
  Manager* mgr = this->manager();
  if (mgr)
    {
    result = mgr->refStorage();
    }
  return result;
}
//----------------------------------------------------------------------------
smtk::attribute::AttributePtr Attribute::pointer() const
{
  Manager *m = this->manager();
  if (m)
    {
    return m->findAttribute(this->m_name);
    }
  return smtk::attribute::AttributePtr();
}
//----------------------------------------------------------------------------
void Attribute::associateEntity(smtk::model::ItemPtr entity)
{
  if (this->isEntityAssociated(entity))
    {
    // Nothing to be done
    return;
    }
  this->m_entities.insert(entity);
  entity->attachAttribute(this->pointer());
}
//----------------------------------------------------------------------------
void Attribute::disassociateEntity(smtk::model::ItemPtr entity, bool reverse)
{
  if (!this->isEntityAssociated(entity))
    {
    // Nothing to be done
    return;
    }

  if (this->m_entities.erase(entity))
    {
    if(reverse)
      {
      entity->detachAttribute(this->pointer());
      }
    }
}
//----------------------------------------------------------------------------
/**\brief Remove all associations of this attribute with model entities.
  *
  * Note that this removes both new- and old-style associations.
  */
void Attribute::removeAllAssociations()
{
  // old-style model entities
  std::set<smtk::model::ItemPtr>::const_iterator it =
    this->associatedEntities();
  for (; it != this->m_entities.end(); it++)
    {
    (*it)->detachAttribute(this->pointer(), false);
    }
  this->m_entities.clear();

  // new-style model entities
  smtk::model::StoragePtr storage;
  unsigned long attribId = this->id();
  if (storage)
    {
    smtk::util::UUIDs::const_iterator mit;
    for (
      mit = this->m_modelEntities.begin();
      mit != this->m_modelEntities.end();
      ++mit)
      {
      storage->detachAttribute(attribId, *mit, false);
      }
    }
  this->m_modelEntities.clear();
}
//----------------------------------------------------------------------------
/**\brief Is the model \a entity associated with this attribute?
  *
  */
bool Attribute::isEntityAssociated(const smtk::util::UUID& entity) const
{
  return (this->m_modelEntities.find(entity) != this->m_modelEntities.end());
}
//----------------------------------------------------------------------------
/**\brief Is the model entity of the \a cursor associated with this attribute?
  *
  */
bool Attribute::isEntityAssociated(const smtk::model::Cursor& cursor) const
{
  return this->isEntityAssociated(cursor.entity());
}
//----------------------------------------------------------------------------
/*! \fn template<typename T> T Attribute::associatedModelEntities() const
 *\brief Return a container of associated cursor-subclass instances.
 *
 * This method returns a container (usually a std::vector or std::set) of
 * cursor-subclass instances (e.g., Edge, EdgeUse, Loop) that are
 * associated with this attribute.
 *
 * Note that if you request a container of Cursor entities, you will obtain
 * <b>all</b> of the associated model entities. However, if you request
 * a container of some subclass, only entities of that type will be returned.
 * For example, if an attribute is associated with two faces, an edge,
 * a group, and a shell, calling `associatedModelEntities<Cursors>()` will
 * return 5 Cursor entries while `associatedModelEntities<CellEntities>()`
 * will return 3 entries (2 faces and 1 edge) since the other entities
 * do not construct valid CellEntity instances.
 */
//----------------------------------------------------------------------------
/**\brief Associate a new-style model ID (a UUID) with this attribute.
  *
  * This function returns true when the association is valid and
  * successful. It may return false if the association is prohibited.
  * (This is not currently implemented.)
  */
bool Attribute::associateEntity(const smtk::util::UUID& entity)
{
  if (this->isEntityAssociated(entity))
    {
    // Nothing to be done
    return true; // Entity may be and is now associated
    }
  // TODO: Verify that entity may be associated with this attribute.
  //       If it may not, then we should return false.
  this->m_modelEntities.insert(entity);
  smtk::model::StoragePtr storage = this->modelStorage();
  if (storage)
    storage->attachAttribute(this->id(), entity);
  return true; // Entity may be and is now associated.
}
//----------------------------------------------------------------------------
/**\brief Disassociate a new-style model ID (a UUID) from this attribute.
  *
  */
void Attribute::disassociateEntity(const smtk::util::UUID& entity, bool reverse)
{
  if (!this->isEntityAssociated(entity))
    {
    // Nothing to be done
    return;
    }

  this->m_modelEntities.erase(entity);
  if(reverse)
    {
    smtk::model::StoragePtr storage = this->modelStorage();
    if (storage)
      {
      storage->detachAttribute(this->id(), entity, false);
      }
    }
}
//----------------------------------------------------------------------------
smtk::attribute::ConstItemPtr Attribute::find(const std::string &inName) const
{
  int i = this->m_definition->findItemPosition(inName);
  if (i < 0)
    {
    return smtk::attribute::ConstItemPtr();
    }
  return this->m_items[i];
}

//----------------------------------------------------------------------------
smtk::attribute::ItemPtr Attribute::find(const std::string &inName)
{
  int i = this->m_definition->findItemPosition(inName);
  return (i < 0) ? smtk::attribute::ItemPtr() : this->m_items[i];
}
//-----------------------------------------------------------------------------
