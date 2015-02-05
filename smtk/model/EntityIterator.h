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

#include "smtk/io/ExportJSON.h" // for JSONRecords

namespace smtk {
  namespace model {

class SMTKCORE_EXPORT EntityIterator
{
public:
  template<typename C> void traverse(C begin, C end);
  template<typename C> void traverse(C begin, C end, smtk::io::JSONRecords style);

  void traverse(const EntityRef& x);
  void traverse(const EntityRef& x, smtk::io::JSONRecords style);

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
  smtk::io::JSONRecords m_related;
};

/**\brief Iterate over the given entities and **only** those entities.
  */
template<typename C>
void EntityIterator::traverse(C begin, C end)
{
  this->traverse(begin, end, smtk::io::JSON_BARE);
}

/**\brief Iterate over the given entities and the specified \a related records.
  */
template<typename C>
void EntityIterator::traverse(C begin, C end, smtk::io::JSONRecords related)
{
  this->m_related = related;
  this->m_visited.clear();
  if (this->m_related == smtk::io::JSON_MODELS)
    {
    Model parent;
    for (C rit = begin; rit != end; ++rit)
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
    this->m_visited.insert(begin, end);
    }
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_EntityIterator_h
