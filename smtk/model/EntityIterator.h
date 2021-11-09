//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_EntityIterator_h
#define smtk_model_EntityIterator_h

#include "smtk/model/Resource.h"

namespace smtk
{
namespace model
{

/**\brief Indicate what records should be visited.
  *
  */
enum IteratorStyle
{
  ITERATE_BARE = 0,     //!< Visit only the specified entities and no others
  ITERATE_CHILDREN = 1, //!< Visit the specified entities and their children.
  ITERATE_MODELS =
    2 //!< Visit all entities with an owning model that also owns any of the specified entities. Also visits the owning session, but not all of the owning session's models.
};

class SMTKCORE_EXPORT EntityIterator
{
public:
  template<typename C>
  void traverse(C ebegin, C eend);
  template<typename C>
  void traverse(C ebegin, C eend, IteratorStyle style);

  void traverse(const EntityRef& x);
  void traverse(const EntityRef& x, IteratorStyle style);

  void begin();
  // This method has no implementation!
  void end();
  bool advance();
  bool isAtEnd() const;

  EntityRef operator++();
  EntityRef operator++(int i);

  EntityRef operator*() const;
  const EntityRef* operator->() const;
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
  m_related = related;
  m_visited.clear();
  if (m_related == ITERATE_MODELS)
  {
    Model parent;
    for (C rit = ebegin; rit != eend; ++rit)
    {
      if ((parent = rit->owningModel()).isValid())
      {
        m_visited.insert(parent);
        SessionRef sref = parent.session();
        if (sref.isValid())
          m_visited.insert(sref);
      }
      else if (rit->isModel())
      {
        Model model(*rit);
        SessionRef sref = model.session();
        m_visited.insert(model);
        if (sref.isValid())
          m_visited.insert(sref);
      }
      else
      {
        m_visited.insert(
          *rit); // Well, if it doesn't have a parent, at least make sure it's included.
      }
    }
  }
  else
  {
    m_visited.insert(ebegin, eend);
  }
}

} // namespace model
} // namespace smtk

#endif // smtk_model_EntityIterator_h
