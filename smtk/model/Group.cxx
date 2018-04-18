//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Group.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

namespace smtk
{
namespace model
{

static const std::string memberMaskName("membership mask");

/**\brief Return the parent of this group.
  *
  * The group may be embedded in multiple containers but its first
  * SUBSET_OF arrangement is the one which determines its direct
  * parent.
  */
EntityRef Group::parent() const
{
  return EntityRefArrangementOps::firstRelation<EntityRef>(*this, SUBSET_OF);
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
Group& Group::addEntity(const EntityRef& thing)
{
  ManagerPtr mgr = this->manager();
  if (this->meetsMembershipConstraints(thing))
  {
    mgr->findOrAddEntityToGroup(this->entity(), thing.entity());
    mgr->trigger(
      std::make_pair(ADD_EVENT, GROUP_SUPERSET_OF_ENTITY), *this, EntityRef(mgr, thing.entity()));
  }

  return *this;
}

/**\brief Remove an entity from this group.
  *
  * TODO: Implement constraint-checking and related changes (i.e., if
  * this group is part of a partition, move \a thing out of
  * other groups in the partition so that we maintain "partition-ness."
  */
bool Group::removeEntity(const EntityRef& thing)
{
  ManagerPtr mgr = this->manager();
  if (this->isValid() && !thing.entity().isNull())
  {
    int aidx = mgr->findArrangementInvolvingEntity(this->m_entity, SUPERSET_OF, thing.entity());

    // FIXME: This really belongs inside unarrangeEntity().
    // But there we have no access to thing.entity() until too late.
    if (mgr->unarrangeEntity(this->m_entity, SUPERSET_OF, aidx) > 0)
    {
      mgr->trigger(
        std::make_pair(DEL_EVENT, GROUP_SUPERSET_OF_ENTITY), *this, EntityRef(mgr, thing.entity()));

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
EntityRef Group::findFirstNonGroupMember()
{
  EntityRefs entries = this->members<EntityRefs>();
  for (EntityRefs::iterator it = entries.begin(); it != entries.end(); ++it)
  {
    if (it->isValid())
    {
      if (it->isGroup())
      {
        EntityRef tmp = it->as<Group>().findFirstNonGroupMember();
        if (tmp.isValid())
          return tmp;
      }
      else
      {
        return *it;
      }
    }
  }
  return EntityRef();
}

/**\brief Test whether the \a prospectiveMember should be allowed in the group.
  *
  * Only constraints for this group are tested; if this group is a member of
  * another group with more restrictive constraints, those will not be tested
  * automatically.
  * However, if \a prospectiveMember is itself a group then its members and
  * all of its child groups members are tested.
  */
bool Group::meetsMembershipConstraints(const EntityRef& prospectiveMember)
{
  BitFlags typeMask = 0;
  bool mustBeHomogenous = this->entityFlags() & HOMOGENOUS_GROUP ? true : false;
  return this->meetsMembershipConstraintsInternal(prospectiveMember, typeMask, mustBeHomogenous);
}

/**\brief Set constraints on what may be added as a member of the group.
  *
  * This is stored separately from the Entity record as an
  * integer property named "membership mask".
  *
  * Note that the \a mask is not used directly; bits from
  * the Group::entityFlags() related to membership constraints
  * and dimensionality are bitwise-ORed with the mask.
  */
void Group::setMembershipMask(BitFlags mask)
{
  this->setIntegerProperty(
    memberMaskName, (this->entityFlags() & (ANY_DIMENSION | GROUP_CONSTRAINT_MASK)) | mask);
}

/**\brief Return the mask constraining the types of entities that may be members.
  *
  * Note that because the group's entityFlags() also constrain members,
  * this may not return the exact mask value passed to setMembershipMask();
  * in particular, the group's dimension bits and group constraint bits
  * are bitwise-ORed with the mask.
  */
BitFlags Group::membershipMask() const
{
  BitFlags result = (this->entityFlags() & (ANY_DIMENSION | GROUP_CONSTRAINT_MASK)) | ENTITY_MASK;
  if (this->hasIntegerProperty(memberMaskName))
  {
    const IntegerList& prop(this->integerProperty(memberMaskName));
    if (!prop.empty())
      result = static_cast<BitFlags>(prop[0]);
  }
  return result;
}

/**\brief A protected method used by the public version to handle recursive group tests.
  *
  * The \a typeMask flag is used to pass information about entities contained
  * in \a prospectiveMember to its child groups (if any).
  * The \a mustBeHomogenous flag indicates whether all of the non-group members must be
  * mustBeHomogenous. If true, then the first non-group member encountered will
  * be used to update the bitmask of acceptable entries.
  */
bool Group::meetsMembershipConstraintsInternal(
  const EntityRef& prospectiveMember, BitFlags& typeMask, bool mustBeHomogenous)
{
  BitFlags groupFlags = this->entityFlags();
  BitFlags memberMask;
  if (typeMask)
  {
    memberMask = typeMask;
  }
  else
  {
    memberMask = this->membershipMask();
    // First, if no entity types other than groups are specified, we assume that all
    // entity types are allowed. We do not currently handle the case where you wish
    // to have a group composed of only a hierarchy of groups with no other entities.
    if ((memberMask & ENTITY_MASK) == GROUP_ENTITY)
      memberMask |= ENTITY_MASK;
    // By default, a group can contain other groups as long as their members meet the
    // membership test.
    // We accept shorthand where the mask does not explicitly mention groups being
    // allowed as intending for groups to be allowed as long as they have members
    // that are allowed. TODO: need to watch changes to group membership.
    //    if (!(memberMask & GROUP_ENTITY))
    //      memberMask |= GROUP_ENTITY;
    // OK, there are cases when groups really should be omitted.
    if (groupFlags & NO_SUBGROUPS)
      memberMask &= ~GROUP_ENTITY;
    // Must the group be homogenous? If so, our mask may be constrained by
    // existing members. If there are no pre-existing members, then the
    // prospective member will force the group type.
    if (groupFlags & HOMOGENOUS_GROUP)
    {
      EntityRef preExistingMember = this->findFirstNonGroupMember();
      if (preExistingMember.isValid())
        memberMask &= preExistingMember.entityFlags();
    }
    // Now we look at how the dimensionality of members should be constrained.
    // First, do we require things to match the model dimensionality (or its boundary)?
    if (groupFlags & (MODEL_BOUNDARY | MODEL_DOMAIN))
    {
      BitFlags dimMask;
      unsigned int dimBits;
      Model parentModel = this->owningModel();
      if (parentModel.isValid())
        dimBits = parentModel.dimensionBits();
      else
        throw std::string("Cannot check group membership constraint based on "
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
    if (!(memberMask & ANY_DIMENSION))
      memberMask |= ANY_DIMENSION;

    // Potential early exit: Expected types do not include any valid members of this group.
    if (typeMask && !(typeMask & memberMask))
      return false;
  }

  // Now we can test the prospective member against our mask

  // No easy rejection is possible. We must match at least one allowed entity type
  // and one allowed dimension bit.
  BitFlags memberFlags = prospectiveMember.entityFlags();
  BitFlags memberTest = (memberFlags & memberMask);
  if (
    // The entity has no dimension bits in common with the mask:
    !(memberTest & ANY_DIMENSION) &&
    // The mask has some, but not all dimension bits set:
    ((memberMask & ANY_DIMENSION) != ANY_DIMENSION) && (memberMask & ANY_DIMENSION))
  { // The entity cannot be a member (it is not of the proper dimension)
    return false;
  }
  // If the prospective member has no entity-type bits in common with the mask and the mask has any entity bits set:
  if (!(memberTest & ENTITY_MASK) && (memberMask & ENTITY_MASK))
  { // The entity cannot be a member (it is not of the proper type)
    return false;
  }

  // If the prospectiveMember is a group, we must now also test its members.
  // If it is not a group, maybe we should update the typemask.
  if (memberFlags & GROUP_ENTITY)
  {
    EntityRefs entries = prospectiveMember.as<Group>().members<EntityRefs>();
    for (EntityRefs::iterator it = entries.begin(); it != entries.end(); ++it)
    {
      if (!this->meetsMembershipConstraintsInternal(*it, memberMask, mustBeHomogenous))
        return false;
      // Update the mask if we find a non-group entity.
      if (mustBeHomogenous && it->isValid() && !it->isGroup())
        memberMask &= it->entityFlags();
    }
  }

  return true;
}

} // namespace model
} // namespace smtk
