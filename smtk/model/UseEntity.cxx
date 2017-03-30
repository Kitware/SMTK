//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/UseEntity.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityRefArrangementOps.h"
#include "smtk/model/Manager.h"

namespace smtk
{
namespace model
{

/// Return the cell owning this cell-use.
CellEntity UseEntity::cell() const
{
  return EntityRefArrangementOps::firstRelation<CellEntity>(*this, HAS_CELL);
}

/// Return the first (usually the only) shell in which this use participates as a boundary.
ShellEntity UseEntity::boundingShellEntity() const
{
  return EntityRefArrangementOps::firstRelation<ShellEntity>(*this, HAS_SHELL);
}

/// The orientation of the cell-use relative to the underlying cell.
Orientation UseEntity::orientation() const
{
  // This is tricky. We currently only store the orientation with
  // the cell, not the cell-use. So we must retrieve the particular
  // arrangement on the cell pointing to this Use and report
  // that arrangement's orientation.

  // Find the cell for this use record.
  ManagerPtr mgr = this->manager();
  if (!mgr)
    return UNDEFINED;

  Entity* ent = mgr->findEntity(this->m_entity);
  if (!ent)
    return UNDEFINED;

  const Arrangement* arr = mgr->findArrangement(this->m_entity, HAS_CELL, 0);
  if (ent && arr)
  {
    int idx, esense;
    arr->IndexAndSenseFromUseHasCell(idx, esense);
    smtk::common::UUID cellId = ent->relations()[idx];
    // Now find the cell's HAS_USE record with the same sense as us:
    int arrIdx = mgr->findCellHasUseWithSense(cellId, this->m_entity, esense);
    if (arrIdx >= 0)
    {
      // Now find the orientation of that use of the cell:
      Orientation orient;
      mgr->findArrangement(cellId, HAS_USE, arrIdx)
        ->IndexSenseAndOrientationFromCellHasUse(idx, esense, orient);
      return orient;
    }
  }
  return UNDEFINED;
}

/// Return the sense of the given use with respect to its parent cell.
int UseEntity::sense() const
{
  ManagerPtr mgr = this->manager();
  // Find the cell for this use record.
  Entity* ent = mgr->findEntity(this->m_entity);
  const Arrangement* arr = mgr->findArrangement(this->m_entity, HAS_CELL, 0);
  if (ent && arr)
  {
    int idx, esense;
    arr->IndexAndSenseFromUseHasCell(idx, esense);
    return esense;
  }
  return -1;
}

/**\brief Set the higher-dimensional shell whose boundary this use participates in.
  *
  */
UseEntity& UseEntity::setBoundingShellEntity(const ShellEntity& shell)
{
  ManagerPtr mgr = this->manager();
  if (mgr)
    mgr->findOrAddIncludedShell(this->m_entity, shell.entity());
  return *this;
}

/**\brief Add a lower-dimensional shell to this use.
  *
  * Each shell added should be a top-level shell for the given use;
  * shells may contain other shells.
  * However, currently this is not detected or enforced and the
  * existing sessions do not properly nest shells.
  */
UseEntity& UseEntity::addShellEntity(const ShellEntity& shell)
{
  ManagerPtr mgr = this->manager();
  if (mgr)
    mgr->findOrAddIncludedShell(this->m_entity, shell.entity());
  return *this;
}

/*! \fn UseEntity::boundingShellEntities() const
 * \brief Return the shells whose boundary this entity-use participates in, in a container of the specified type.
 *
 * For all but VertexUse entities, this should return at most one entry
 * (i.e., a given EdgeUse participates in at most one Loop).
 * You should use boundingShellEntity() as a convenience in this case.
 *
 * For VertexUses, the same use may participate in many Chains;
 * otherwise the number of VertexUse entities would be combinatorially huge.
 * One VertexUse entry should exist for each k-cell in the model that
 * does not bound a (k+1)-cell and is bounded by this vertex.
 *
 * For example:<pre>
 *   UseEntity u; // say u.isEdgeUse() == true
 *   Loops uloops = u.boundingShellEntities<Loops>();
 *   // or alternatively:
 *   typedef std::set<Loop> LoopSet;
 *   LoopSet uloops = u.shellEntities<LoopSet>();
 * </pre>
 */

/*! \fn UseEntity::shellEntities() const
 * \brief Return the shells forming the boundary of this entity-use in a container of the specified type.
 *
 * For example:<pre>
 *   UseEntity u; // say u.isFaceUse() == true
 *   Loops uloops = u.shellEntities<Loops>();
 *   // or alternatively:
 *   typedef std::set<Loop> LoopSet;
 *   LoopSet uloops = u.shellEntities<LoopSet>();
 * </pre>
 */

/*! \fn UseEntity::addShellEntities(T& shellContainer)
 * \brief Add the shells in the \a shellContainer to this entity-use (which form its boundary).
 *
 * This calls addShellEntity for each entry in \a shellContainer.
 */

} // namespace model
} // namespace smtk
