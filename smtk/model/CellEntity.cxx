#include "smtk/model/CellEntity.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/CursorArrangementOps.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Storage.h"

namespace smtk {
  namespace model {


/**\brief Return the model owning this cell (or an invalid cursor if the cell is free).
  *
  */
ModelEntity CellEntity::model() const
{
  StoragePtr store = this->storage();
  return ModelEntity(
    store,
    store->modelOwningEntity(this->entity()));
}

/**\brief Report the toplevel shell records associated with the cell.
  *
  * The uses can be used to discover lower-dimensional cells that
  * form the boundary of this cell.
  */
ShellEntities CellEntity::shellEntities() const
{
  ShellEntities result;
  CursorArrangementOps::appendAllRelations(*this, HAS_SHELL, result);
  return result;
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
