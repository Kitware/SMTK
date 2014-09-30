//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/GroupEntity.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Manager.h"
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
  * If the group has any bits set constraining its membership,
  * this method will call meetsMembershipConstraints() to verify
  * that the entity can be added. If meetsMembershipConstraints
  * returns false, the entity will not be added.
  *
  * TODO: Implement constraint-checking and related changes (i.e., if
  * this group is part of a partition, move \a thing out of
  * other groups in the partition so that we maintain "partition-ness."
  */
GroupEntity& GroupEntity::addEntity(const Cursor& thing)
{
  this->m_manager->findOrAddEntityToGroup(this->entity(), thing.entity());
  this->m_manager->trigger(
    std::make_pair(ADD_EVENT,GROUP_SUPERSET_OF_ENTITY),
    *this,
    Cursor(this->m_manager, thing.entity()));

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
    int aidx = this->m_manager->findArrangementInvolvingEntity(
      this->m_entity, SUPERSET_OF, thing.entity());

    // FIXME: This really belongs inside unarrangeEntity().
    // But there we have no access to thing.entity() until too late.
    if (this->m_manager->unarrangeEntity(this->m_entity, SUPERSET_OF, aidx) > 0)
      {
      this->m_manager->trigger(
        std::make_pair(DEL_EVENT,GROUP_SUPERSET_OF_ENTITY),
        *this,
        Cursor(this->m_manager, thing.entity()));

      return true;
      }
    }
  return false;
}

/**\brief Return the first member of this group that is not a group itself.
  *
  * This does a depth-first traversal, so a subgroup's "leaf" entry
  * may be the value returned even if this group has "leaf" entries
  * at its top level.
  *
  * This is used by meetsMembershipConstraints (when the group
  * is constrained to be homogenous) to identify the type of
  * item the group must hold.
  */
Cursor GroupEntity::findFirstNonGroupMember()
{
  Cursors entries = this->members<Cursors>();
  for (Cursors::iterator it = entries.begin(); it != entries.end(); ++it)
    {
    if (it->isValid())
      {
      if (it->isGroupEntity())
        {
        Cursor tmp = it->as<GroupEntity>().findFirstNonGroupMember();
        if (tmp.isValid())
          return tmp;
        }
      else
        {
        return *it;
        }
      }
    }
  return Cursor();
}

/**\brief Test whether the \a prospectiveMember should be allowed in the group.
  *
  * Only constraints for this group are tested; if this group is a member of
  * another group with more restrictive constraints, those will not be tested
  * automatically.
  * However, if \a prospectiveMember is itself a group then its members and
  * all of its child groups members are tested.
  */
bool GroupEntity::meetsMembershipConstraints(
  const Cursor& prospectiveMember)
{
  BitFlags typeMask = 0;
  bool mustBeHomogenous = this->entityFlags() & HOMOGENOUS_GROUP ? true : false;
  return this->meetsMembershipConstraintsInternal(
    prospectiveMember, typeMask, mustBeHomogenous);
}

/**\brief A protected method used by the public version to handle recursive group tests.
  *
  * The \a typeMask flag is used to pass information about entities contained
  * in \a prospectiveMember to its child groups (if any).
  * The \a mustBeHomogenous flag indicates whether all of the non-group members must be
  * mustBeHomogenous. If true, then the first non-group member encountered will
  * be used to update the bitmask of acceptable entries.
  */
bool GroupEntity::meetsMembershipConstraintsInternal(
  const Cursor& prospectiveMember,
  BitFlags& typeMask,
  bool mustBeHomogenous)
{
  BitFlags groupFlags = this->entityFlags();
  BitFlags memberMask;
  if (typeMask)
    {
    memberMask = typeMask;
    }
  else
    {
    memberMask = groupFlags;
    // First, if no entity types other than groups are specified, we assume that all
    // entity types are allowed. We do not currently handle the case where you wish
    // to have a group composed of only a hierarchy of groups with no other entities.
    if ((groupFlags & GROUP_ENTITY) == GROUP_ENTITY) memberMask |= ENTITY_MASK;
    // By default, a group can contain other groups as long as their members meet the
    // membership test.
    if (groupFlags & NO_SUBGROUPS) memberMask &= ~GROUP_ENTITY;
    // Must the group be homogenous? If so, our mask may be constrained by
    // existing members. If there are no pre-existing members, then the
    // prospective member will force the group type.
    if (groupFlags & HOMOGENOUS_GROUP)
      {
      Cursor preExistingMember = this->findFirstNonGroupMember();
      if (preExistingMember.isValid())
        memberMask &= preExistingMember.entityFlags();
      }
    // Now we look at how the dimensionality of members should be constrained.
    // First, do we require things to matcht he model dimensionality (or its boundary)?
    if (groupFlags & (MODEL_BOUNDARY | MODEL_DOMAIN))
      {
      BitFlags dimMask;
      unsigned int dimBits;
      ModelEntity parentModel = this->owningModel();
      if (parentModel.isValid())
        dimBits = parentModel.dimensionBits();
      else
        throw std::string(
          "Cannot check group membership constraint based on "
          "model dimensionality because it has no model.");
      if ((dimBits != 0) & ((dimBits & (dimBits - 1)) == 0))
        {
        // If the model dimension is a power of two,
        // then we know the model and boundary dimensions.
        dimMask = (groupFlags == MODEL_DOMAIN ? dimBits : (dimBits >> 1));
        }
      else
        {
        dimMask = dimBits;
        // Otherwise, we must find the most-significant bit and adjust:
        int maxDim = 0;
        while (dimMask >>= 1)
          ++maxDim;
        dimMask = (1 << maxDim);
        if (groupFlags == MODEL_BOUNDARY)
          dimMask >>= 1;
        }
      // Only accept entities with the appropriate domain or boundary bit set.
      memberMask &= ((~ANY_DIMENSION) | dimMask);
      }
    // We assume that no dimension bits set means that any dimension should be allowed.
    // Otherwise, no model entities except perhaps instances would be allowed (and there
    // is another way to achieve this -- set the INSTANCE_ENTITY bit)
    if (!(memberMask & ANY_DIMENSION)) memberMask |= ANY_DIMENSION;

    // Potential early exit: Expected types do not include any valid members of this group.
    if (typeMask && !(typeMask & memberMask))
      return false;
    }

  // Now we can test the prospective member against our mask

  // No easy rejection is possible. We must match at least one allowed entity type
  // and one allowed dimension bit.
  BitFlags memberFlags = prospectiveMember.entityFlags();
  BitFlags memberTest = (memberFlags & memberMask);
  if (!(memberTest & ANY_DIMENSION) && (memberMask & ANY_DIMENSION)) return false;
  if (!(memberTest & ANY_ENTITY) && (memberMask & ANY_ENTITY)) return false;

  // If the prospectiveMember is a group, we must now also test its members.
  // If it is not a group, maybe we should update the typemask.
  if (memberFlags & GROUP_ENTITY)
    {
    Cursors entries = prospectiveMember.as<GroupEntity>().members<Cursors>();
    for (Cursors::iterator it = entries.begin(); it != entries.end(); ++it)
      {
      if (!this->meetsMembershipConstraintsInternal(*it, memberMask, mustBeHomogenous))
        return false;
      // Update the mask if we find a non-group entity.
      if (mustBeHomogenous && it->isValid() && !it->isGroupEntity())
        memberMask &= it->entityFlags();
      }
    }

  return true;
}

  } // namespace model
} // namespace smtk
