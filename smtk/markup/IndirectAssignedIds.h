//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_IndirectAssignedIds_h
#define smtk_markup_IndirectAssignedIds_h

#include "smtk/markup/AssignedIds.h"
#include "smtk/markup/IdType.h"

#include "smtk/common/Visit.h"

#include "vtkIdTypeArray.h"
#include "vtkSmartPointer.h"

namespace smtk
{
namespace markup
{

/// IDs assigned in random order by an array held on a component.
///
/// Because the order of IDs in the component is not consecutive,
/// this class assumes there is an array (of the proper size for
/// the component) holding IDs as they appear. There may be
/// unused IDs within the range, however no IDs outside the
/// range() of the assignment may appear.
class SMTKMARKUP_EXPORT IndirectAssignedIds : public AssignedIds
{
public:
  smtkTypeMacro(smtk::markup::IndirectAssignedIds);
  smtkSuperclassMacro(smtk::markup::AssignedIds);

  template<typename... Args>
  IndirectAssignedIds(
    const std::shared_ptr<IdSpace>& space,
    IdNature nature,
    IdType begin,
    IdType end,
    Component* owningNode,
    Args&&... /*args*/)
    : AssignedIds(space, nature, begin, end, owningNode)
  {
  }

  ~IndirectAssignedIds() override = default;

  using AssignedIds::AssignedIdCtor;
  using AssignedIds::Forwardness;

  /// Provide a constructor functor to be passed to IdSpace::requestRange().
  AssignedIdCtor cloneFunctor() const override;

  /// Set/get the array of IDs used in the order they are indexed by the component.
  void setIdArray(vtkSmartPointer<vtkIdTypeArray> idArray);
  vtkSmartPointer<vtkIdTypeArray> idArray() const { return m_idArray; }

  /**\brief An iterator for array-indexed IDs.
    *
    * We introduce a new type rather than using the underlying
    * multimap iterator to harmonize the iterator API with
    * SequentialAssignedIds, which has an index() method.
    *
    * This iterator may provide multiple index values per ID
    * (since the array may repeat an ID) but this use case is
    * expected to be rare; an additional method is provided to
    * fetch them all rather than the first occurrence.
    */
  template<Forwardness IsForward>
  struct Iterator
  {
    using value_type = IdType;
    using reference_type = IdType&;
    using index_type = vtkIdType;

    static constexpr IdType Invalid = ~0ull;

    Iterator(const Iterator<IsForward>& other) = default;
    ~Iterator() = default;

    Iterator<IsForward>& operator=(const Iterator<IsForward>& other) = default;

    Iterator<IsForward> operator++()
    {
      Iterator<IsForward> result = *this;
      auto parent = m_parent.lock();
      if (!parent || m_value == Invalid)
      {
        return result;
      }
      m_value =
        (IsForward == Forward ? this->advance(m_value, parent) : this->retreat(m_value, parent));
      return result;
    }

    Iterator<IsForward>& operator++(int)
    {
      if (m_value == Invalid)
      {
        return *this;
      }
      auto parent = m_parent.lock();
      if (!parent)
      {
        return *this;
      }
      m_value =
        (IsForward == Forward ? this->advance(m_value, parent) : this->retreat(m_value, parent));
      return *this;
    }

    Iterator<IsForward> operator--()
    {
      Iterator<IsForward> result = *this;
      auto parent = m_parent.lock();
      if (!parent || m_value == Invalid)
      {
        return result;
      }
      m_value =
        (IsForward == Forward ? this->retreat(m_value, parent) : this->advance(m_value, parent));
      return result;
    }

    Iterator<IsForward>& operator--(int)
    {
      if (m_value == Invalid)
      {
        return *this;
      }
      auto parent = m_parent.lock();
      if (!parent)
      {
        return *this;
      }
      m_value =
        (IsForward == Forward ? this->retreat(m_value, parent) : this->advance(m_value, parent));
      return *this;
    }

    Iterator& operator+=(std::size_t count)
    {
      if (m_value == Invalid)
      {
        return *this;
      }
      auto parent = m_parent.lock();
      if (!parent)
      {
        return *this;
      }
      m_value =
        (IsForward == Forward ? this->advance(m_value, parent, count)
                              : this->retreat(m_value, parent, count));
      return *this;
    }

    Iterator& operator-=(std::size_t count)
    {
      if (m_value == Invalid)
      {
        return *this;
      }
      auto parent = m_parent.lock();
      if (!parent)
      {
        return *this;
      }
      m_value =
        (IsForward == Forward ? this->retreat(m_value, parent, count)
                              : this->advance(m_value, parent, count));
      return *this;
    }

    Iterator<IsForward> operator+(std::size_t delta) const
    {
      Iterator<IsForward> result = *this;
      result += delta;
      return result;
    }

    Iterator<IsForward> operator-(std::size_t delta) const
    {
      Iterator<IsForward> result = *this;
      result -= delta;
      return result;
    }

    const value_type& operator*() const { return m_value; }

    const value_type& operator->() const { return m_value; }

    /// Return the index of the ID within this AssignedIds object.
    ///
    /// The index is the ID minus range()[0].
    /// For example, if a 10×10×10 image has point IDs 100 through 1100,
    /// the iterator's value (the result of dereferencing the iterator
    /// using the '*' operator) might be 101 while the index would be 1.
    /// This is useful for translating from IDs to (i,j,k)-indices in the
    /// image.
    index_type index() const
    {
      auto parent = m_parent.lock();
      if (!parent || m_value == Invalid)
      {
        return Invalid;
      }
      return parent->firstIndexFor(m_value);
    }

    bool operator==(const Iterator& other) const
    {
      // NB: We could also test m_parent == other.m_parent and raise an exception.
      return m_value == other.m_value;
    }
    bool operator!=(const Iterator& other) const
    {
      // NB: We could also test m_parent == other.m_parent and raise an exception.
      return m_value != other.m_value;
    }

  protected:
    friend class IndirectAssignedIds;
    Iterator(IdType value, std::shared_ptr<const IndirectAssignedIds> parent)
      : m_value(value)
      , m_parent(parent)
    {
    }

    value_type advance(
      value_type current,
      const std::shared_ptr<const IndirectAssignedIds>& parent,
      std::size_t count = 1) const
    {
      value_type result = current;
      while (count > 0)
      {
        auto it = parent->m_idToIndex.upper_bound(result);
        if (it == parent->m_idToIndex.end())
        {
          return Invalid;
        }
        result = it->first;
        --count;
      }
      return result;
    }

    value_type retreat(
      value_type current,
      const std::shared_ptr<const IndirectAssignedIds>& parent,
      std::size_t count = 1) const
    {
      value_type result = current;
      while (count > 0)
      {
        auto it = parent->m_idToIndex.lower_bound(result);
        if (it == parent->m_idToIndex.end() || it == parent->m_idToIndex.begin())
        {
          result = Invalid;
          return result;
        }
        --it;
        result = it->first;
        --count;
      }
      return result;
    }

    value_type m_value;
    std::weak_ptr<const IndirectAssignedIds> m_parent;
  };

  /// Return the number of allotted IDs.
  /// This simply returns range()[1] - range()[0], since the range is a half-open interval.
  IdType size() const;
  /// Return the number of alloted IDs less one (as a convenience for iteration).
  IdType maxId() const;
  /// Return true when no IDs are assigned.
  bool empty() const;

  /// Return an iterator pointing to the first assigned ID.
  Iterator<Forward> begin() const;
  /// Return an iterator just past the end of the last assigned ID.
  Iterator<Forward> end() const;

  /// Return an iterator pointing to the last assigned ID.
  Iterator<Reverse> rbegin() const;
  /// Return an iterator just before the beginning of the first assigned ID.
  Iterator<Reverse> rend() const;

  /// Return the number of allotted ids in the half-open interval [begin, end[.
  ///
  /// Note that this does not count repeated ids more than once.
  /// For instance, if the underlying array holds 1, 3, 3, 5, 7, then
  /// contains(1, 4) == 2 (and contains(1, 5) == 2 as well since \a end
  /// is not included in the count).
  IdType contains(IdType begin, IdType end) const;

  /// Call \a visitor on each alloted ID.
  template<typename Functor>
  smtk::common::Visited visit(Functor visitor) const
  {
    smtk::common::VisitorFunctor<Functor> f(visitor);
    smtk::common::Visited didVisit = smtk::common::Visited::Empty;
    for (const auto& id : *this)
    {
      if (f(id) == smtk::common::Visit::Halt)
      {
        return smtk::common::Visited::Some;
      }
      didVisit = smtk::common::Visited::All;
    }
    return didVisit;
  }

  vtkIdType firstIndexFor(IdType value) const;

protected:
  vtkSmartPointer<vtkIdTypeArray> m_idArray;
  std::multimap<IdType, vtkIdType> m_idToIndex;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_IndirectAssignedIds_h
