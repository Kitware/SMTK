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

  } // namespace model
} // namespace smtk
