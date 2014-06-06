#ifndef __smtk_model_GroupEntity_h
#define __smtk_model_GroupEntity_h

#include "smtk/model/Cursor.h"
#include "smtk/model/CursorArrangementOps.h" // for templated methods

namespace smtk {
  namespace model {

class GroupEntity;
typedef std::vector<GroupEntity> GroupEntities;

/**\brief A cursor subclass that provides methods specific to entity use records.
  *
  */
class SMTKCORE_EXPORT GroupEntity : public Cursor
{
public:
  SMTK_CURSOR_CLASS(GroupEntity,Cursor,isGroupEntity);

  Cursor parent() const;
  template<typename T> T members() const;

  GroupEntity& addEntity(const Cursor& entity);
  template<typename T> GroupEntity& addEntities(const T& container);

  bool removeEntity(const Cursor& entity);

  Cursor findFirstNonGroupMember();

  virtual bool meetsMembershipConstraints(
    const Cursor& prospectiveMember);

protected:
  friend class smtk::attribute::ModelEntityItemDefinition;

  bool meetsMembershipConstraintsInternal(
    const Cursor& prospectiveMember,
    BitFlags& typeMask,
    bool mustBeHomogenous);
};

template<typename T>
T GroupEntity::members() const
{
  T container;
  CursorArrangementOps::appendAllRelations(*this, SUPERSET_OF, container);
  return container;
}

/// Add all the entities in \a container (an STL set, vector, or list) to this group.
template<typename T>
GroupEntity& GroupEntity::addEntities(const T& container)
{
  for (typename T::const_iterator it = container.begin(); it != container.end(); ++it)
    {
    this->addEntity(*it);
    }
  return *this;
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_GroupEntity_h
