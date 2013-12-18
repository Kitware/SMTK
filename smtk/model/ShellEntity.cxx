#include "smtk/model/ShellEntity.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/CursorArrangementOps.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Storage.h"
#include "smtk/model/UseEntity.h"

namespace smtk {
  namespace model {

/**\brief Return the high-dimensional cell whose interior is bounded by this shell.
  *
  */
CellEntity ShellEntity::parentCell() const
{
  (void)this;
  return CursorArrangementOps::firstRelation<CellEntity>(*this, HAS_CELL);
}

/**\brief Return the uses (cells with an orientation, or sense) composing this shell.
  *
  * When the shell is 1-dimensional, these uses will be ordered curve segments
  * that match head-to-tail and describe a closed loop.
  *
  * TODO: Decide how to handle multiple closed loops: should an "empty" shell (that
  * returns uses().empty()) be created with multiple containedShells()? Or should
  * shells be allowed to have siblings?
  */
UseEntities ShellEntity::uses() const
{
  UseEntities result;
  CursorArrangementOps::appendAllRelations(*this, HAS_USE, result);
  return result;
}

ShellEntities ShellEntity::containedShells() const
{
  ShellEntities result;
  CursorArrangementOps::appendAllRelations(*this, HAS_SHELL, result);
  return result;
}

  } // namespace model
} // namespace smtk
