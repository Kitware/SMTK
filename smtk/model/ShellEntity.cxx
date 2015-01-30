//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/ShellEntity.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/CursorArrangementOps.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/UseEntity.h"

namespace smtk {
  namespace model {

/**\brief Return the high-dimensional cell whose interior is bounded by this shell.
  *
  */
CellEntity ShellEntity::boundingCell() const
{
  return this->boundingUseEntity().cell();
}

/**\brief Return the high-dimensional cell-use whose interior is bounded by this shell.
  *
  * This method will return the proper use even if this shell
  * is a subshell of another (and thus not directly related to
  * the cell-use).
  */
UseEntity ShellEntity::boundingUseEntity() const
{
  Cursor result = CursorArrangementOps::firstRelation<Cursor>(*this, EMBEDDED_IN);
  while (result.isValid() && result.isShellEntity())
    {
    result = result.as<ShellEntity>().containingShellEntity();
    }
  // Is the top-level shell contained directly in a cell?
  // (This is the case for volume cells but not faces or edges.)
  // FIXME: Should this be removed? It doesn't match the pattern
  //        SMTK adopts with top-level shells being EMBEDDED_IN a use.
  if (result.isValid() && result.isCellEntity())
    return result.as<CellEntity>().uses<UseEntities>()[0];
  // The top-level containing entity should be a use of a cell.
  return result.as<UseEntity>();
}

/**\brief Return the shell-entity containing this one (or an invalid shell-entity if unbounded).
  *
  */
ShellEntity ShellEntity::containingShellEntity() const
{
  return CursorArrangementOps::firstRelation<ShellEntity>(*this, EMBEDDED_IN);
}

/// Add the (lower-dimensional) use as a child of the shell.
ShellEntity& ShellEntity::addUse(const UseEntity& use)
{
  ManagerPtr mgr = this->manager();
  if (mgr)
    mgr->findOrAddUseToShell(this->m_entity, use.entity());
  return *this;
}

  } // namespace model
} // namespace smtk
