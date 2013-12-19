#include "smtk/model/UseEntity.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Storage.h"
#include "smtk/model/Arrangement.h"
#include "smtk/model/CursorArrangementOps.h"

namespace smtk {
  namespace model {

CellEntity UseEntity::cell() const
{
  return CursorArrangementOps::firstRelation<CellEntity>(*this, HAS_CELL);
}

/*! \fn UseEntity::shellsAs() const
 * \brief Return the shells that contain this entity-use in a container of the specified type.
 *
 * For example:<pre>
 *   UseEntity u;
 *   Loops uloops = u.shellsAs<Loops>();
 *   // or alternatively:
 *   typedef std::set<Loop> LoopSet;
 *   LoopSet uloops = u.shellsAs<LoopSet>();
 * </pre>
 */

  } // namespace model
} // namespace smtk
