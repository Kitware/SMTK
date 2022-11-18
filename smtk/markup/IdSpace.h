//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_IdSpace_h
#define smtk_markup_IdSpace_h

#include "smtk/markup/Domain.h"

#include "smtk/markup/IdNature.h"
#include "smtk/markup/IdType.h"

#include "boost/icl/split_interval_map.hpp"

#include <array>

namespace smtk
{
namespace markup
{

class AssignedIds;

/**\brief A numbering used in a context.
  *
  * Identifier spaces hold reservations of IDs assigned to components in a given
  * context using a data structure that allows for fast queries across ranges of IDs.
  *
  * Examples of id spaces include: node numbers, element numbers, side numbers, pedigree IDs, global IDs.
  */
class SMTKMARKUP_EXPORT IdSpace : public smtk::markup::Domain
{
public:
  smtkTypeMacro(smtk::markup::IdSpace);
  smtkSuperclassMacro(smtk::markup::Domain);

  /// The plain-old-data type used to hold an identifier.
  using IdType = smtk::markup::IdType;
  /// The search structure used to index assignments in the space of IDs.
  using IntervalTree = boost::icl::split_interval_map<IdType, std::set<AssignedIds*>>;
  /// A structure that simplifies determining whether assignments cover a range.
  using IntervalMerge = boost::icl::interval_set<IdType>;
  /// Signature of a function to create an assigned-IDs object.
  using AssignedIdCtor = std::function<
    std::shared_ptr<AssignedIds>(const std::shared_ptr<IdSpace>&, IdNature, IdType, IdType)>;

  /// A constant used to indicate an invalid ID.
  static constexpr IdType Invalid = ~0ull;

  IdSpace() = default;
  IdSpace(smtk::string::Token name);
  IdSpace(const nlohmann::json& data);
  ~IdSpace() override = default;

  const std::array<IdSpace::IdType, 2>& range() const;
  std::array<IdSpace::IdType, 2>& range();

  /// Create a default instance of AssignedIds from a specification.
  static shared_ptr<AssignedIds> defaultAssignment(
    const std::shared_ptr<IdSpace>& space,
    IdNature nature,
    IdType begin,
    IdType end);

  /**\brief Request access to or control of a range of IDs.
    *
    * The \a nature of the request determines conditions under
    * which the request may fail.
    *
    * + Referential requests fail if the range is empty
    *   (you cannot reference what does not exist).
    * + NonExclusive requests fail if the range contains IDs
    *   assigned for exclusive, primary access.
    * + Primary requests fail if the range is non-empty.
    *
    * Upon failure, a null pointer is returned.
    * Upon success, an instance of AssignedIds is constructed
    * and a weak pointer is held by this IdSpace; a shared
    * pointer is returned and must be held by the node
    * using the requested IDs.
    * When the node is deleted, the AssignedIds
    * instance will be deleted and its destructor will
    * remove it from this IdSpace instance.
    *
    * The \a offset specifies the starting ID in the request.
    * An \a offset *must* be provided for Referential requests.
    * An \a offset *may* be provided for requests of other \a natures.
    *
    * If \a offset is specified, then the request will
    * fail if the range [offset, offset + rangeSize] does
    * not meet the conditions required by \a nature.
    * Otherwise, a starting value for the range will be
    * chosen to satisfy the requirements if that is possible.
    *
    * If a \a ctor (constructor) is specified, then it will
    * be invoked to construct the output AssignedIds instance.
    * This is used by discrete geometry subclasses (ImageData
    * and UnstructuredData) to create SequentialAssignedIds
    * or IndirectAssignedIds as needed so that they can provide
    * reverse-lookups between IDs and the index of the corresponding
    * primitive or point. Note that AssignedIds::node() will return
    * a null pointer unless the \a ctor passes a a non-null value.
    * The \a defaultAssignment does not.
    */
  std::shared_ptr<AssignedIds> requestRange(
    IdNature nature,
    std::size_t rangeSize,
    std::size_t offset = Invalid,
    AssignedIdCtor ctor = &IdSpace::defaultAssignment);

  /**\brief Insert assigned IDs into the space without checks.
    *
    * This method is only intended for deserialization.
    * It accepts an externally-created \a assignedIds instance
    * and adds it to the interval tree.
    * It will only accept \a assignedIds if its ID space is a
    * reference to this instance.
    */
  void insertAssignment(const AssignedIds* assignedIds);

  /**\brief Return all ID assignments of the given \a nature overlapping [ \a begin , \a end [.
    *
    * If \a nature is Unassigned (the default), this returns all ID assignments regardless of
    * their nature. Otherwise, only the requested subset is returned.
    */
  std::set<std::shared_ptr<AssignedIds>>
  assignedIds(IdType begin, IdType end, IdNature nature = IdNature::Unassigned) const;

  /** Test whether the half-open interval from \a begin to \a end has any assigned IDs.
    *
    * If \a nature is Unassigned (the default), this returns true if the range has no
    * assignments at all and false otherwise.
    *
    * If \a nature is Primary, then this returns true if the range is empty of *both*
    * Primary and NonExclusive assignments – and false otherwise.
    *
    * If \a nature is NonExclusive, then this returns true if the range is empty
    * of NonExclusive assignments and false othewise.
    *
    * If \a nature is Referential, then this returns true if the range is empty
    * of Referential assignments and false otherwise.
    */
  bool isRangeEmpty(IdType begin, IdType end, IdNature allowed = IdNature::Unassigned) const;

  /**\brief Test whether the half-open interval from \a begin to \a end is covered
    *       by assigned IDs of the given \a nature.
    *
    * If \a nature is Unassigned (the default), this returns true if the range is covered
    * by any assignments at all and false otherwise.
    *
    * If \a nature is Primary, then this returns true if the range is covered by *any combination*
    * of Primary and NonExclusive assignments – and false otherwise.
    *
    * If \a nature is NonExclusive, then this returns true if the range is covered by
    * NonExclusive assignments and false othewise.
    *
    * If \a nature is Referential, then this returns true if the range is covered by
    * Referential assignments and false otherwise.
    */
  bool isRangeCovered(IdType begin, IdType end, IdNature allowed = IdNature::Unassigned) const;

  /// If the half-open interval from \a begin to \a end has any primary IDs.
  bool rangeHasPrimaryIds(IdType begin, IdType end) const;

  /**\brief Return the number IDs in [ \a begin, \a end [ with one or more
    *       assignments of the given \a nature.
    *
    * If \a nature is IdNature::Unassigned, then the number returned counts
    * IDs without *any* assignment.
    */
  IdType numberOfIdsInRangeOfNature(
    IdType begin,
    IdType end,
    IdNature nature = IdNature::Unassigned) const;

protected:
  friend class AssignedIds;

  /// This method is called by the AssignedIds destructor to unregister itself from m_entries.
  bool removeEntry(const AssignedIds& entry);

  static std::array<IdSpace::IdType, 2>
  clampedRange(const std::array<IdType, 2>& unclamped, IdType begin, IdType end);

  std::array<IdSpace::IdType, 2> m_range{ Invalid, Invalid };

  /**\brief A container to hold all the assigned IDs within the space.
    *
    * The container is indexed by the lowest ID within the assignment.
    */
  IntervalTree m_entries;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_IdSpace_h
