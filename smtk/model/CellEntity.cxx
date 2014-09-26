//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/CellEntity.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/CursorArrangementOps.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/UseEntity.h"

#include <deque>

namespace smtk {
  namespace model {


/**\brief Return the model owning this cell (or an invalid cursor if the cell is free).
  *
  */
ModelEntity CellEntity::model() const
{
  ManagerPtr mgr = this->manager();
  return ModelEntity(
    mgr,
    mgr->modelOwningEntity(this->entity()));
}

/**\brief Report all of the lower-dimensional cells bounding this cell.
  *
  * The bounding cells are obtained by traversing all of the shells of
  * the first use of this cell.
  */
CellEntities CellEntity::boundingCells() const
{
  CellEntities cells;
  UseEntities useEnts = this->uses<UseEntities>();
  if (!useEnts.empty())
    {
    std::deque<ShellEntity> tmp =
      useEnts[0].shellEntities<std::deque<ShellEntity> >();
    ShellEntity shell;
    for (; !tmp.empty(); tmp.pop_front())
      {
      shell = tmp.front();
      // Add the shell's use's cells to the output
      UseEntities su = shell.uses<UseEntities>();
      UseEntities::iterator uit;
      for (uit = su.begin(); uit != su.end(); ++uit)
        {
        CellEntity ce = uit->cell();
        if (ce.isValid())
          cells.insert(cells.end(), ce);
        }
      // Add the shell's inner shells to tmp
      ShellEntities innerShells =
        shell.containedShellEntities<ShellEntities>();
      ShellEntities::iterator iit;
      for (iit = innerShells.begin(); iit != innerShells.end(); ++iit)
        tmp.push_back(*iit);
      }
    }
  // FIXME: This should be: "else if (this->dimension() > 0)" once
  //        the CGM bridge properly transcribes use entities.
  if (cells.empty() && this->dimension() > 0)
    { // Try harder... see if bordantEntities reveals anything
    Cursors bdys = this->boundaryEntities(this->dimension() - 1);
    Cursors::const_iterator it;
    for (it = bdys.begin(); it != bdys.end(); ++it)
      { // Only add valid cells
      if (it->as<CellEntity>().isValid())
        {
        cells.insert(cells.end(), *it);
        }
      }
    }
  return cells; // or CellEntities(cells.begin(), cells.end()); if cells is a set.
}

/*! \fn CellEntity::inclusions() const
 * \brief Return the list of all entities embedded in this cell.
 *
 * Note that the inverse of this relation is provided by
 * Cursor::embeddedIn().
 */

/*! \fn CellEntity::uses() const
 * \brief Report all of the "use" records associated with the cell.
 *
 * The uses can be used to discover higher-dimensional cells that
 * this cell borders.
 * Each sense of a cell has its own use.
 */

  } // namespace model
} // namespace smtk
