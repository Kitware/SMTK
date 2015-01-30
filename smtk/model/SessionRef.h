//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_SessionRef_h
#define __smtk_model_SessionRef_h

#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/EntityRefArrangementOps.h" // for templated methods

namespace smtk {
  namespace model {

class SessionRef;
typedef std::vector<SessionRef> SessionRefs;

/**\brief A entityref subclass that provides methods specific to entity use records.
  *
  */
class SMTKCORE_EXPORT SessionRef : public EntityRef
{
public:
  SMTK_ENTITYREF_CLASS(SessionRef,EntityRef,isSessionRef);
  SessionRef(ManagerPtr manager, SessionPtr brdg);

  SessionPtr session() const;

  template<typename T> T models() const;

  StringList operatorNames() const;
  smtk::attribute::System* opSys() const;
  OperatorDefinition opDef(const std::string& opName) const;
  OperatorPtr op(const std::string& opName) const;

  StringList operatorsForAssociation(BitFlags assocMask) const;
  template<typename T>
  StringList operatorsForAssociation(const T& entityrefContainer) const;

  std::string tag() const;
  std::string site() const;
  StringList engines() const;
  StringData fileTypes(
    const std::string& engine = std::string()) const;
};

template<typename T>
T SessionRef::models() const
{
  T container;
  ManagerPtr mgr = this->m_manager.lock();
  if (!mgr || !this->m_entity)
    return container;

  smtk::common::UUIDs mids = mgr->modelsOfSession(this->m_entity);
  smtk::common::UUIDs::iterator it;
  for (it = mids.begin(); it != mids.end(); ++it)
    {
    typename T::value_type entry(mgr, *it);
    if (entry.isValid())
      container.insert(container.end(), entry);
    }
  return container;
}

template<typename T>
StringList SessionRef::operatorsForAssociation(const T& entityrefContainer) const
{
  BitFlags mask = ANY_ENTITY;
  typename T::const_iterator it;
  for (it = entityrefContainer.begin(); mask && it != entityrefContainer.end(); ++it)
    {
    mask &= it->entityFlags();
    }
  return this->operatorsForAssociation(mask);
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_SessionRef_h
