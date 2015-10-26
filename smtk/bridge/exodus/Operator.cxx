//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/exodus/Operator.h"
#include "smtk/bridge/exodus/Session.h"

#include "vtkDataObject.h"

namespace smtk {
  namespace bridge {
    namespace exodus {

/// Return a shared pointer to the session backing a Exodus operator.
Session* Operator::exodusSession()
{
  return dynamic_cast<smtk::bridge::exodus::Session*>(this->session());
}

/**\brief A helper to return the Exodus data object associated with an \a smtkEntity.
  *
  */
vtkDataObject* Operator::exodusData(const smtk::model::EntityRef& smtkEntity)
{
  Session* brdg = this->exodusSession();
  if (!brdg)
    return NULL;

  return brdg->toEntity(smtkEntity).object<vtkDataObject>();
}

/**\brief A helper to return the Exodus handle associated with an \a smtkEntity.
  *
  */
EntityHandle Operator::exodusHandle(const smtk::model::EntityRef& smtkEntity)
{
  Session* brdg = this->exodusSession();
  if (!brdg)
    return EntityHandle();

  return brdg->toEntity(smtkEntity);
}

    } // namespace exodus
  } //namespace bridge
} // namespace smtk
