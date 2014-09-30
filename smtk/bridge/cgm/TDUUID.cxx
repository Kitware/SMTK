//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/TDUUID.h"
#include "smtk/bridge/cgm/CAUUID.h"

#include "CubitAttrib.hpp"
#include "CubitAttribUser.hpp"
#include "ToolDataUser.hpp"

#include <sstream>

namespace smtk {
  namespace bridge {
    namespace cgm {

UUIDToCGMRef TDUUID::s_reverseLookup;
smtk::common::UUIDGenerator TDUUID::s_uuidGenerator;

/**\brief Attach an SMTK UUID (\a uid) to the CGM \a entity.
  *
  * This will throw an exception if \a uid is already in use.
  *
  * The default value for \a uid is a null UUID (which is invalid).
  * If an invalid UUID is passed to the constructor, a random
  * one that is not already in use (according to s_reverseLookup)
  * will be assigned to the \a entity.
  */
TDUUID::TDUUID(ToolDataUser* entity, const smtk::common::UUID& uid)
  : m_entityId(uid)
{
  if (this->m_entityId.isNull())
    {
    while (
      TDUUID::s_reverseLookup.find(
        (this->m_entityId = TDUUID::s_uuidGenerator.random())) !=
      TDUUID::s_reverseLookup.end())
      /* keep generating new UUIDs */;
    }
  else
    {
    // This may throw an exception. If it doesn't, we are OK to proceed.
    TDUUID::checkForCollision(entity, uid);
    }

  entity->add_TD(this);
  TDUUID::s_reverseLookup[this->m_entityId] = entity;

  // Update the CubitAttrib if the entity may be cast to it.
  CubitAttribUser* cau = dynamic_cast<CubitAttribUser*>(entity);
  if (cau)
    {
    CubitAttrib* attrib = cau->get_cubit_attrib(CA_UUID);
    attrib->has_updated(CUBIT_FALSE);
    attrib->update();
    }
}

TDUUID::~TDUUID()
{
  UUIDToCGMRef::iterator it = TDUUID::s_reverseLookup.find(this->m_entityId);
  if (it != TDUUID::s_reverseLookup.end())
    {
    // TODO: Signal SMTK that an entity is disappearing.
    TDUUID::s_reverseLookup.erase(it);
    }
}

/// Return the SMTK UUID associated with this ToolData (attached to a CGM entity).
smtk::common::UUID TDUUID::entityId() const
{
  return this->m_entityId;
}

/**\brief Generate the ToolData that should be put on \a new_td_user when this entity is split.
  *
  */
ToolData* TDUUID::propogate(ToolDataUser* new_td_user)
{
  // TODO: Signal SMTK that an entity is appearing.
  return new TDUUID(new_td_user, TDUUID::s_uuidGenerator.random());
}

/**\brief Generate the ToolData for the ToolDataUser when this CGM entity and \a other_td_user are merged.
  *
  * Currently this returns NULL because we are not passed the ToolDataUser.
  */
ToolData* TDUUID::merge(ToolDataUser* other_td_user)
{
  (void)other_td_user;
  // TODO: Signal SMTK that two entities (this->m_entity, other_td_user's entityId()) are merging.
  return NULL;
}

/// Find the CGM entity associated with the given \a uid. May return NULL.
ToolDataUser* TDUUID::findEntityById(const smtk::common::UUID& uid)
{
  UUIDToCGMRef::const_iterator it = TDUUID::s_reverseLookup.find(uid);
  return it == TDUUID::s_reverseLookup.end() ? NULL : it->second;
}

/**\brief Returns true if the given ToolData instance \a td is an instance of TDUUID.
  *
  * This is used by TDUUID::ofEntity().
  */
int TDUUID::isTDUUID(const ToolData* td)
{
  return dynamic_cast<const TDUUID*>(td) == NULL ? 0 : 1;
}

/**\brief Return the TDUUID object associated with the given \a entity.
  *
  * If \a createNew is true, this should only return NULL if \a entity is NULL.
  * When \a createNew is false, this will also return NULL when no
  * TDUUID is associated with the given CGM \a entity.
  */
TDUUID* TDUUID::ofEntity(ToolDataUser* entity, bool createNew)
{
  if (!entity) return NULL;
  TDUUID* result = static_cast<TDUUID*>(entity->get_TD(&TDUUID::isTDUUID));
  if (!result && createNew)
    {
    result = new TDUUID(entity);
    }
  return result;
}

/**\brief Throw an exception if the UUID already exists in the reverse lookup.
  *
  */
void TDUUID::checkForCollision(ToolDataUser* entity, const smtk::common::UUID& uid)
{
  UUIDToCGMRef::const_iterator it = TDUUID::s_reverseLookup.find(uid);
  // See if either the entity already has the same TDUUID or there's a collision.
  if (it != TDUUID::s_reverseLookup.end())
    {
    std::ostringstream errMsg;
    if (it->second == entity)
      errMsg << "Duplicate TDUUID " << uid << " created for entity " << entity;
    else
      errMsg << "TDUUID collision. Both " << it->second << " and " << entity << " cannot have ID " << uid;
    throw errMsg.str();
    }
}

} // namespace cgm
  } //namespace bridge
} // namespace smtk
