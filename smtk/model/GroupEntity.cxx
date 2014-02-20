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
  this->m_storage->findOrAddEntityToGroup(this->entity(), thing.entity());
  return *this;
}

/**\brief Remove an entity from this group.
  *
  * TODO: Implement constraint-checking and related changes (i.e., if
  * this group is part of a partition, move \a thing out of
  * other groups in the partition so that we maintain "partition-ness."
  */
bool GroupEntity::removeEntity(const Cursor& thing)
{
  if (this->isValid() && !thing.entity().isNull())
    {
    int aidx = this->m_storage->findArrangementInvolvingEntity(
      this->m_entity, SUPERSET_OF, thing.entity());
    if (this->m_storage->unarrangeEntity(this->m_entity, SUPERSET_OF, aidx) > 0)
      return true;
    }
  return false;
}

  } // namespace model
} // namespace smtk
