//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_BridgeSession_h
#define __smtk_model_BridgeSession_h

#include "smtk/model/Manager.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/CursorArrangementOps.h" // for templated methods

namespace smtk {
  namespace model {

class BridgeSession;
typedef std::vector<BridgeSession> BridgeSessions;

/**\brief A cursor subclass that provides methods specific to entity use records.
  *
  */
class SMTKCORE_EXPORT BridgeSession : public Cursor
{
public:
  SMTK_CURSOR_CLASS(BridgeSession,Cursor,isBridgeSession);
  BridgeSession(ManagerPtr manager, BridgePtr brdg);

  BridgePtr bridge() const;

  template<typename T> T models() const;

  StringList operatorNames() const;
  smtk::attribute::System* opSys() const;
  OperatorDefinition opDef(const std::string& opName) const;
  OperatorPtr op(const std::string& opName) const;

  StringList operatorsForAssociation(BitFlags assocMask) const;
  template<typename T>
  StringList operatorsForAssociation(const T& cursorContainer) const;

  std::string tag() const;
  std::string site() const;
  StringList engines() const;
  StringData fileTypes(
    const std::string& engine = std::string()) const;
};

template<typename T>
T BridgeSession::models() const
{
  T container;
  if (!this->m_manager || !this->m_entity)
    return container;

  smtk::common::UUIDs mids = this->m_manager->modelsOfBridgeSession(this->m_entity);
  smtk::common::UUIDs::iterator it;
  for (it = mids.begin(); it != mids.end(); ++it)
    {
    typename T::value_type entry(this->m_manager, *it);
    if (entry.isValid())
      container.insert(container.end(), entry);
    }
  return container;
}

template<typename T>
StringList BridgeSession::operatorsForAssociation(const T& cursorContainer) const
{
  BitFlags mask = ANY_ENTITY;
  typename T::const_iterator it;
  for (it = cursorContainer.begin(); mask && it != cursorContainer.end(); ++it)
    {
    mask &= it->entityFlags();
    }
  return this->operatorsForAssociation(mask);
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_BridgeSession_h
