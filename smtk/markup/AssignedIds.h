//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_AssignedIds_h
#define smtk_markup_AssignedIds_h

#include "smtk/SharedFromThis.h"
#include "smtk/markup/Exports.h"
#include "smtk/markup/IdNature.h" // for IdNature
#include "smtk/markup/IdType.h"   // for IdType

#include "smtk/string/Token.h"

#include "smtk/common/Visit.h"

namespace smtk
{
namespace markup
{

class Component;
class IdSpace;

/// An API for querying the IDs allotted to a component in an IdSpace.
class SMTKMARKUP_EXPORT AssignedIds : public std::enable_shared_from_this<AssignedIds>
{
public:
  smtkTypeMacroBase(smtk::markup::AssignedIds);

  using IdType = smtk::markup::IdType;
  using IdIterator = struct
  {
    smtk::markup::IdSpace* idSpace;
    IdType begin;
    IdType end;
    IdNature nature;
  };
  using IdRange =
    std::array<IdType, 2>; // struct { smtk::markup::IdSpace* idSpace; IdType begin; IdType end; };
  using AssignedIdCtor = std::function<
    std::shared_ptr<AssignedIds>(const std::shared_ptr<IdSpace>&, IdNature, IdType, IdType)>;

  template<typename... Args>
  AssignedIds(
    const std::shared_ptr<IdSpace>& space,
    IdNature nature,
    IdType begin,
    IdType end,
    Component* owningNode,
    Args&&... /*args*/)
    : m_space(space)
    , m_range(IdRange{ begin, end })
    , m_nature(nature)
    , m_node(owningNode)
  {
  }

  virtual ~AssignedIds();

  /// Subclasses must override this method to return a functor producing an instance of themselves.
  virtual AssignedIdCtor cloneFunctor() const;

  /// The access-level of iteration (constant or mutable), used as a template parameter.
  // enum Constness { Const, NonConst }; // All iterators must be constant or IDs are not sequential.

  /// The direction of iteration (used as a template parameter for subclass iterators).
  enum Forwardness
  {
    Forward, //!< The iterator traverses IDs from lowest to highest.
    Reverse  //!< The iterator traverses IDs from highest to lowest.
  };

  /// Returns the range of IDs in the allotment.
  IdRange range() const;
  /// Return the number of allotted IDs.
  /// This simply returns range()[1] - range()[0], since the range is a half-open interval.
  IdType size() const { return m_range[1] - m_range[0]; }
  /// Return true when no IDs are assigned.
  bool empty() const { return this->size() > 0; }

  std::shared_ptr<smtk::markup::IdSpace> space() const;

  bool setNature(const IdNature& nature);
  const IdNature& nature() const;
  IdNature& nature();

  Component* node() const { return m_node; }
  template<typename NodeType>
  NodeType* nodeAs() const
  {
    return dynamic_cast<NodeType*>(m_node);
  }

protected:
  /// The parent space from which our assigned IDs come.
  std::weak_ptr<smtk::markup::IdSpace> m_space;
  /// The overall range of the assignment (used to accelerate intersection tests).
  IdRange m_range;
  /// The nature of the assignment (ownership or reference; exclusive or non-exclusive).
  IdNature m_nature;
  /// The graph node which owns the assignment (if any).
  Component* m_node;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_AssignedIds_h
