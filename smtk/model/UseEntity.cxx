#include "smtk/model/UseEntity.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Storage.h"
#include "smtk/model/Arrangement.h"
#include "smtk/model/CursorArrangementOps.h"

namespace smtk {
  namespace model {

/// Return the cell owning this cell-use.
CellEntity UseEntity::cell() const
{
  return CursorArrangementOps::firstRelation<CellEntity>(*this, HAS_CELL);
}

/// Return the first (usually the only) shell in which this use participates as a boundary.
ShellEntity UseEntity::boundingShellEntity() const
{
  return CursorArrangementOps::firstRelation<ShellEntity>(*this, HAS_SHELL);
}

/// The orientation of the cell-use relative to the underlying cell.
Orientation UseEntity::orientation() const
{
  // This is tricky. We currently only store the orientation with
  // the cell, not the cell-use. So we must retrieve the particular
  // arrangement on the cell pointing to this FaceUse and report
  // that arrangement's orientation.

  // Find the cell for this use record.
  Entity* ent = this->m_storage->findEntity(this->m_entity);
  const Arrangement* arr = this->m_storage->findArrangement(
    this->m_entity, HAS_CELL, 0);
  if (ent && arr)
    {
    int idx, esense;
    arr->IndexAndSenseFromUseHasCell(idx, esense);
    smtk::util::UUID cellId = ent->relations()[idx];
    // Now find the cell's HAS_USE record with the same sense as us:
    int arrIdx = this->m_storage->findCellHasUseWithSense(cellId, esense);
    if (arrIdx >= 0)
      {
      // Now find the orientation of that use of the cell:
      Orientation orient;
      this->m_storage->findArrangement(cellId, HAS_USE, arrIdx)
        ->IndexSenseAndOrientationFromCellHasUse(idx, esense, orient);
      return orient;
      }
    }
  return UNDEFINED;
}

/// Return the sense of the given use with respect to its parent cell.
int UseEntity::sense() const
{
  // Find the cell for this use record.
  Entity* ent = this->m_storage->findEntity(this->m_entity);
  const Arrangement* arr = this->m_storage->findArrangement(
    this->m_entity, HAS_CELL, 0);
  if (ent && arr)
    {
    int idx, esense;
    arr->IndexAndSenseFromUseHasCell(idx, esense);
    return esense;
    }
  return -1;
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

  } // namespace model
} // namespace smtk
