//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Group_h
#define __smtk_model_Group_h

#include "smtk/model/EntityRef.h"
#include "smtk/model/EntityRefArrangementOps.h" // for templated methods

namespace smtk {
  namespace model {

class Group;
typedef std::vector<Group> Groups;

/**\brief A entityref subclass that provides methods specific to entity use records.
  *
  */
class SMTKCORE_EXPORT Group : public EntityRef
{
public:
  SMTK_ENTITYREF_CLASS(Group,EntityRef,isGroup);

  EntityRef parent() const;
  template<typename T> T members() const;

  Group& addEntity(const EntityRef& entity);
  template<typename T> Group& addEntities(const T& container);

  bool removeEntity(const EntityRef& entity);

  EntityRef findFirstNonGroupMember();

  virtual bool meetsMembershipConstraints(
    const EntityRef& prospectiveMember);

  virtual void setMembershipMask(BitFlags mask);
  BitFlags membershipMask() const;

protected:
  friend class smtk::attribute::ModelEntityItemDefinition;

  bool meetsMembershipConstraintsInternal(
    const EntityRef& prospectiveMember,
    BitFlags& typeMask,
    bool mustBeHomogenous);
};

template<typename T>
T Group::members() const
{
  T container;
  EntityRefArrangementOps::appendAllRelations(*this, SUPERSET_OF, container);
  return container;
}

/// Add all the entities in \a container (an STL set, vector, or list) to this group.
template<typename T>
Group& Group::addEntities(const T& container)
{
  for (typename T::const_iterator it = container.begin(); it != container.end(); ++it)
    {
    this->addEntity(*it);
    }
  return *this;
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Group_h
