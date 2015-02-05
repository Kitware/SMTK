//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/EntityIterator.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Group.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/UseEntity.h"

namespace smtk {
  namespace model {

void EntityIterator::traverse(const EntityRef& x)
{
  this->traverse(x, smtk::io::JSON_CHILDREN);
}

void EntityIterator::traverse(const EntityRef& x, smtk::io::JSONRecords style)
{
  this->m_related = style;
  this->m_visited.clear();
  if (this->m_related == smtk::io::JSON_MODELS)
    {
    Model parent;
    if ((parent = x.owningModel()).isValid())
      {
      this->m_visited.insert(parent);
      SessionRef sref = parent.session();
      if (sref.isValid())
        this->m_visited.insert(sref);
      }
    else if (x.isModel())
      {
      Model model(x);
      SessionRef sref = model.session();
      this->m_visited.insert(model);
      if (sref.isValid())
        this->m_visited.insert(sref);
      }
    else
      {
      this->m_visited.insert(x); // Well, if it doesn't have a parent, at least make sure it's included.
      }
    }
  else
    {
    this->m_visited.insert(x);
    }
}

void EntityIterator::begin()
{
  this->m_queue = this->m_visited;
  this->m_visited.clear();
}

/// Advance to the next item, returning true until reaching the end of iteration.
bool EntityIterator::advance()
{
  ++(*this);
  return !this->isAtEnd();
}

/// Return true when iteration complete, false otherwise.
bool EntityIterator::isAtEnd() const
{
  return this->m_queue.empty();
}

/**\brief Prefix increment operator.
  *
  * This advances the iterator and returns the newly-updated current item.
  */
EntityRef EntityIterator::operator ++ ()
{
  if (this->isAtEnd())
    return EntityRef();

  EntityRefs::iterator it = this->m_queue.begin();
  this->m_visited.insert(*it);
  // Always call after inserting argument into m_visited to prevent stupidity:
  this->updateQueue(*it);
  this->m_queue.erase(it);
  return this->isAtEnd() ?
    EntityRef() :
    *this->m_queue.begin();
}

/**\brief Postfix increment operator.
  *
  * This advances the iterator but returns the item that
  * was current before advancing the iterator.
  */
EntityRef EntityIterator::operator ++ (int i)
{
  if (this->isAtEnd())
    return EntityRef();

  EntityRef result = *this->m_queue.begin();
  this->m_visited.insert(result);
  // Always call after inserting argument into m_visited to prevent stupidity:
  this->updateQueue(result);
  this->m_queue.erase(this->m_queue.begin());
  return result;
}

/// A convenience that returns this->current().
EntityRef EntityIterator::operator * () const
{
  return this->current();
}

/// A convenience that returns this->current().
const EntityRef* EntityIterator::operator -> () const
{
  return &this->current();
}

/// Return the current value of the iterator (or an invalid cursor).
const EntityRef& EntityIterator::current() const
{
  static EntityRef dummy;
  return this->isAtEnd() ?
    dummy :
    *this->m_queue.begin();
}

/**\brief Add entities related to \a ent to the queue as required by the style.
  *
  * Used by next and the increment-operators to append items to the queue when
  * iterating over models for the first time.
  */
void EntityIterator::updateQueue(const EntityRef& ent)
{
  switch (this->m_related)
    {
  case smtk::io::JSON_CHILDREN:
  case smtk::io::JSON_MODELS:
      {
      EntityRefs children;
      if (ent.isCellEntity())
        {
        children = ent.boundaryEntities();
        }
      else if (ent.isUseEntity())
        {
        children = ent.as<UseEntity>().shellEntities<EntityRefs>();
        }
      else if (ent.isShellEntity())
        {
        children = ent.as<ShellEntity>().uses<EntityRefs>();
        }
      else if (ent.isGroup())
        {
        children = ent.as<Group>().members<EntityRefs>();
        }
      else if (ent.isModel())
        { // Grrr.... too much cruft.
        CellEntities mcells = ent.as<smtk::model::Model>().cells();
        children.insert(mcells.begin(), mcells.end());

        Groups mgroups = ent.as<smtk::model::Model>().groups();
        children.insert(mgroups.begin(), mgroups.end());

        Models msubmodels = ent.as<smtk::model::Model>().submodels();
        children.insert(msubmodels.begin(), msubmodels.end());
        }
      for (EntityRefs::const_iterator cit = children.begin(); cit != children.end(); ++cit)
        if (this->m_visited.find(*cit) == this->m_visited.end())
          this->m_queue.insert(*cit);
      }
    break;
  default:
    // Do nothing.
    break;
    }
  if (this->m_queue.empty() && this->m_related != smtk::io::JSON_BARE)
    this->m_related = smtk::io::JSON_BARE; // No need to recompute things to visit next time around.
}

  } // namespace model
} // namespace smtk
