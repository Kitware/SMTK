#include "smtk/model/InstanceEntity.h"

#include "smtk/model/CursorArrangementOps.h"
#include "smtk/model/Storage.h"
#include "smtk/model/Arrangement.h"

namespace smtk {
  namespace model {

Cursor InstanceEntity::prototype() const
{
  return CursorArrangementOps::firstRelation<Cursor>(*this, INSTANCE_OF);
}

  } // namespace model
} // namespace smtk
