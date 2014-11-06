//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/Operator.h"
#include "smtk/bridge/cgm/Bridge.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "RefEntity.hpp"

namespace smtk {
  namespace bridge {
    namespace cgm {

/// Return a shared pointer to the bridge backing a CGM operator.
Bridge* Operator::cgmBridge()
{
  return dynamic_cast<smtk::bridge::cgm::Bridge*>(this->bridge());
}

/**\brief A helper to return the CGM ToolDataUser associated with \a smtkEntity.
  *
  * The ToolDataUser is the simplest instance of anything
  * that can be directly associated with a UUID in CGM.
  * Normally you will be more interested in Operator::cgmEntity
  * as it returns the result of this method dynamically cast to
  * the much more useful RefEntity.
  */
ToolDataUser* Operator::cgmData(const smtk::model::Cursor& smtkEntity)
{
  return TDUUID::findEntityById(smtkEntity.entity());
}

/// A helper to return the CGM entity associated with \a smtkEntity.
RefEntity* Operator::cgmEntity(const smtk::model::Cursor& smtkEntity)
{
  ToolDataUser* tdu = TDUUID::findEntityById(smtkEntity.entity());
  RefEntity* ent = dynamic_cast<RefEntity*>(tdu);
  return ent;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk
