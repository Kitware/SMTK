#include "smtk/model/InstanceEntity.h"

#include "smtk/model/CursorArrangementOps.h"
#include "smtk/model/Storage.h"
#include "smtk/model/Arrangement.h"

namespace smtk {
  namespace model {

Cursor InstanceEntity::parent() const
{
  return CursorArrangementOps::firstRelation<Cursor>(*this, EMBEDDED_IN);
}

  } // namespace model
} // namespace smtk
