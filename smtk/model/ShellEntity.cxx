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

/**\brief Return the shell-entity containing this one (or an invalid shell-entity if unbounded).
  *
  */
ShellEntity ShellEntity::containingShellEntity() const
{
  return CursorArrangementOps::firstRelation<ShellEntity>(*this, EMBEDDED_IN);
}

  } // namespace model
} // namespace smtk
