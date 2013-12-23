#include "smtk/model/CellEntity.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/CursorArrangementOps.h"
#include "smtk/model/Storage.h"

namespace smtk {
  namespace model {

/**\brief Report all of the "use" records associated with the cell.
  *
  * The uses can be used to discover higher-dimensional cells that
  * this cell borders.
  * Each sense of a cell has its own use.
  */
UseEntities CellEntity::uses() const
{
  UseEntities result;
  CursorArrangementOps::appendAllRelations(*this, HAS_USE, result);
  return result;
}

/**\brief Report the toplevel shell records associated with the cell.
  *
  * The uses can be used to discover lower-dimensional cells that
  * form the boundary of this cell.
  */
ShellEntities CellEntity::shells() const
{
  ShellEntities result;
  CursorArrangementOps::appendAllRelations(*this, HAS_SHELL, result);
  return result;
}

  } // namespace model
} // namespace smtk
