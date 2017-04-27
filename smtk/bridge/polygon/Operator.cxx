//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/polygon/Operator.h"
#include "smtk/bridge/polygon/Session.h"

#include "smtk/model/EntityRef.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{

/// Return a shared pointer to the session backing a polygon operator.
Session* Operator::polygonSession()
{
  return dynamic_cast<smtk::bridge::polygon::Session*>(this->session());
}

const Session* Operator::polygonSession() const
{
  return dynamic_cast<const smtk::bridge::polygon::Session*>(this->session());
}

void Operator::addStorage(
  const smtk::common::UUID& uid, smtk::bridge::polygon::internal::entity::Ptr storage)
{
  this->polygonSession()->addStorage(uid, storage);
}

bool Operator::removeStorage(const smtk::common::UUID& uid)
{
  return this->polygonSession()->removeStorage(uid);
}

/*
/// A helper to return the polygon entity associated with \a smtkEntity.
internal::Entity* Operator::polygonEntity(const smtk::model::EntityRef& smtkEntity)
{
  ToolDataUser* tdu = TDUUID::findEntityById(smtkEntity.entity());
  RefEntity* ent = dynamic_cast<RefEntity*>(tdu);
  return ent;
}
*/

} // namespace polygon
} //namespace bridge
} // namespace smtk
