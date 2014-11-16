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
#include "smtk/bridge/exodus/Bridge.h"

#include "vtkDataObject.h"

namespace smtk {
  namespace bridge {
    namespace exodus {

/// Return a shared pointer to the bridge backing a Exodus operator.
Bridge* Operator::exodusBridge()
{
  return dynamic_cast<smtk::bridge::exodus::Bridge*>(this->bridge());
}

/**\brief A helper to return the Exodus data object associated with an \a smtkEntity.
  *
  */
vtkDataObject* Operator::exodusData(const smtk::model::Cursor& smtkEntity)
{
  Bridge* brdg = this->exodusBridge();
  if (!brdg)
    return NULL;

  return brdg->toBlock<vtkDataObject>(brdg->toEntity(smtkEntity));
}

    } // namespace exodus
  } //namespace bridge
} // namespace smtk
