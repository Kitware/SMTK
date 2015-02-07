//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_EntityIterator_h
#define __smtk_model_EntityIterator_h

#include "smtk/model/Manager.h"

namespace smtk {
  namespace model {

/**\brief Indicate what records should be visited.
  *
  */
enum IteratorStyle
{
  ITERATE_BARE     = 0, //!< Visit only the specified entities and no others
  ITERATE_CHILDREN = 1, //!< Visit the specified entities and their children.
  ITERATE_MODELS   = 2  //!< Visit all entities with an owning model that also owns any of the specified entities. Also visits the owning session, but not all of the owning session's models.
};

class SMTKCORE_EXPORT EntityIterator
{
public:
  template<typename C> void traverse(C ebegin, C eend);
  template<typename C> void traverse(C ebegin, C eend, IteratorStyle style);

  void traverse(const EntityRef& x);
  void traverse(const EntityRef& x, IteratorStyle style);

  void begin();
  bool advance();
  bool isAtEnd() const;

  EntityRef operator ++ ();
  EntityRef operator ++ (int i);

  EntityRef operator * () const;
  const EntityRef* operator -> () const;
  const EntityRef& current() const;

protected:
  void updateQueue(const EntityRef& ent);

  EntityRefs m_queue;
  EntityRefs m_visited;
  IteratorStyle m_related;
};

/**\brief Iterate over the given entities and **only** those entities.
  */
template<typename C>
void EntityIterator::traverse(C ebegin, C eend)
{
  this->traverse(ebegin, eend, ITERATE_BARE);
}

/**\brief Iterate over the given entities and the specified \a related records.
  */
template<typename C>
void EntityIterator::traverse(C ebegin, C eend, IteratorStyle related)
{
  this->m_related = related;
  this->m_visited.clear();
  if (this->m_related == ITERATE_MODELS)
    {
    Model parent;
    for (C rit = ebegin; rit != eend; ++rit)
      {
      if ((parent = rit->owningModel()).isValid())
        {
        this->m_visited.insert(parent);
        SessionRef sref = parent.session();
        if (sref.isValid())
          this->m_visited.insert(sref);
        }
      else if (rit->isModel())
        {
        Model model(*rit);
        SessionRef sref = model.session();
        this->m_visited.insert(model);
        if (sref.isValid())
          this->m_visited.insert(sref);
        }
      else
        {
        this->m_visited.insert(*rit); // Well, if it doesn't have a parent, at least make sure it's included.
        }
      }
    }
  else
    {
    this->m_visited.insert(ebegin, eend);
    }
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_EntityIterator_h
