#include "smtk/model/GroupEntity.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Storage.h"
#include "smtk/model/Arrangement.h"

namespace smtk {
  namespace model {

/**\brief Return the parent of this group.
  *
  * The group may be embedded in multiple containers but its first
  * SUBSET_OF arrangement is the one which determines its direct
  * parent.
  */
Cursor GroupEntity::parent() const
{
  return CursorArrangementOps::firstRelation<Cursor>(*this, SUBSET_OF);
}

/**\brief Add an entity to this group.
  *
  * TODO: Implement constraint-checking and related changes (i.e., if
  * this group is part of a partition, move \a thing out of
  * other groups in the partition so that we maintain "partition-ness."
  */
GroupEntity& GroupEntity::addEntity(const Cursor& thing)
{
  CursorArrangementOps::findOrAddSimpleRelationship(*this, SUPERSET_OF, thing);
  CursorArrangementOps::findOrAddSimpleRelationship(thing, SUBSET_OF, *this);
  return *this;
}

  } // namespace model
} // namespace smtk
