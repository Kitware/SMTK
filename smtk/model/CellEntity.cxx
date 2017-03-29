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
#include "smtk/model/EntityRefArrangementOps.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/UseEntity.h"

#include <deque>

namespace smtk {
  namespace model {


/**\brief Return the model owning this cell (or an invalid entityref if the cell is free).
  *
  */
Model CellEntity::model() const
{
  ManagerPtr mgr = this->manager();
  return Model(
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
  //        the CGM session properly transcribes use entities.
  if (cells.empty() && this->dimension() > 0)
    { // Try harder... see if bordantEntities reveals anything
    EntityRefs bdys = this->boundaryEntities(this->dimension() - 1);
    EntityRefs::const_iterator it;
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

/**\brief Return oriented uses of boundary cells.
  *
  * This method takes a cell-use of this cell with the given
  * \a orientation, fetches all of that use's shell entities and appends all
  * of those shells' cell-uses to the output.
  *
  * For example, calling boundingCellUses(POSITIVE) on a face will return
  * a set of edge uses that define the boundary of the face.
  *
  * Note that a Volume may only have a POSITIVE orientation and vertices
  * have no boundary.
  */
UseEntities CellEntity::boundingCellUses(Orientation orientation) const
{
  UseEntity cellUse;
  UseEntities result = this->uses<UseEntities>();
  for (UseEntities::iterator it = result.begin(); it != result.end(); ++it)
    if (it->isValid() && it->orientation() == orientation)
      cellUse = *it;
  result.clear();
  if (!cellUse.isValid())
    return result;
  // We have a properly-oriented use; ask for all of its loops, appending
  // each loop's HAS_USE relations to result as we go.
  std::deque<ShellEntity> tmp =
    cellUse.shellEntities<std::deque<ShellEntity> >();
  ShellEntity shell;
  for (; !tmp.empty(); tmp.pop_front())
    {
    shell = tmp.front();
    // Add the shell's use's cells to the output
    UseEntities su = shell.uses<UseEntities>();
    result.insert(result.end(), su.begin(), su.end());
    // Add the shell's inner shells to tmp
    ShellEntities innerShells =
      shell.containedShellEntities<ShellEntities>();
    ShellEntities::iterator iit;
    for (iit = innerShells.begin(); iit != innerShells.end(); ++iit)
      tmp.push_back(*iit);
    }
  return result;
}

/**\brief Return the shell entity bounding this cell which contains \a bdyUse.
  *
  * This can be used to determine (for cells with holes) which boundary
  * segment contains a given orientation and sense of a given bounding cell.
  * For example, you may ask a volume for the shell containing a face-use.
  * You can then ask the shell for its sense relative to the cell and its
  * parent (either the cell if it is an outer shell or another shell if it
  * is an inner shell).
  */
ShellEntity CellEntity::findShellEntityContainingUse(const UseEntity& bdyUse)
{
  UseEntities cellUses = this->uses<smtk::model::UseEntities>();
  for (UseEntities::iterator uit = cellUses.begin(); uit != cellUses.end(); ++uit)
    {
    ShellEntities shellsOfUse = uit->boundingShellEntities<ShellEntities>();
    for (ShellEntities::iterator sit = shellsOfUse.begin(); sit != shellsOfUse.end(); ++sit)
      {
      if (sit->contains(bdyUse))
        {
        return *sit;
        }
      ShellEntities innerShells = sit->containedShellEntities<ShellEntities>();
      for (ShellEntities::iterator iit = innerShells.begin(); iit != innerShells.end(); ++iit)
        {
        if (iit->contains(bdyUse))
          {
          return *iit;
          }
        }
      }
    }
  // Didn't find anything. Return an invalid shell:
  return ShellEntity();
}

/**\brief Return the shell entity bounding this cell which contains \a useEnt.
  *
  * Return all of the shells bounding this cell which reference the given \a cell.
  * This obtains all the uses of \a cell and collects the shell entities
  * for each using CellEntity::findShellEntityContainingUse().
  */
ShellEntities CellEntity::findShellEntitiesContainingCell(const CellEntity& cell)
{
  ShellEntities result;
  UseEntities uses = cell.uses<UseEntities>();
  for (UseEntities::iterator it = uses.begin(); it != uses.end(); ++it)
    {
    ShellEntity shellEnt = this->findShellEntityContainingUse(*it);
    if (shellEnt.isValid())
      {
      result.push_back(shellEnt);
      }
    }
  return result;
}

/*! \fn CellEntity::inclusions() const
 * \brief Return the list of all entities embedded in this cell.
 *
 * Note that the inverse of this relation is provided by
 * EntityRef::embeddedIn().
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
