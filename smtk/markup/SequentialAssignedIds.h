//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_SequentialAssignedIds_h
#define smtk_markup_SequentialAssignedIds_h

#include "smtk/markup/AssignedIds.h"
#include "smtk/markup/IdType.h"

#include "smtk/common/Visit.h"

namespace smtk
{
namespace markup
{

/// IDs assigned in an ordered sequence to a component.
///
/// Because the IDs are sequential, the component does not
/// need to store them directly – only an offset between
/// the index into the component and the starting ID is required.
class SMTKMARKUP_EXPORT SequentialAssignedIds : public AssignedIds
{
public:
  smtkTypeMacro(smtk::markup::SequentialAssignedIds);
  smtkSuperclassMacro(smtk::markup::AssignedIds);

  template<typename... Args>
  SequentialAssignedIds(
    const std::shared_ptr<IdSpace>& space,
    IdNature nature,
    IdType begin,
    IdType end,
    Component* owningNode,
    Args&&... /*args*/)
    : AssignedIds(space, nature, begin, end, owningNode)
  {
  }

  ~SequentialAssignedIds() override = default;

  using AssignedIds::Forwardness;

  /**\brief An iterator for simple, monotonically-increasing IDs.
    *
    */
  template<Forwardness IsForward>
  struct Iterator
  {
    using value_type = IdType;
    using reference_type = IdType&;

    static constexpr IdType Invalid = ~0ull;

    Iterator(const Iterator<IsForward>& other) = default;
    ~Iterator() = default;

    Iterator<IsForward>& operator=(const Iterator<IsForward>& other) = default;

    Iterator<IsForward> operator++()
    {
      Iterator<IsForward> result = *this;
      if (m_value == Invalid)
      {
        return result;
      }
      else if (
        (IsForward == Forwardness::Forward && m_value == m_range[1] - 1) ||
        (IsForward == Forwardness::Reverse && m_value == m_range[0]))
      {
        m_value = Invalid;
      }
      else
      {
        m_value += (IsForward == Forwardness::Forward ? 1 : -1);
      }
      return result;
    }

    Iterator<IsForward>& operator++(int)
    {
      if (m_value == Invalid)
      {
        return *this;
      }
      else if (
        (IsForward == Forwardness::Forward && m_value == m_range[1] - 1) ||
        (IsForward == Forwardness::Reverse && m_value == m_range[0]))
      {
        m_value = Invalid;
      }
      else
      {
        m_value += (IsForward == Forwardness::Forward ? 1 : -1);
      }
      return *this;
    }

    Iterator<IsForward> operator--()
    {
      Iterator<IsForward> result = *this;
      if (m_value == Invalid)
      {
        return result;
      }
      else if (
        (IsForward == Forwardness::Forward && m_value == m_range[0]) ||
        (IsForward == Forwardness::Reverse && m_value == m_range[1] - 1))
      {
        m_value = Invalid;
      }
      else
      {
        m_value -= (IsForward == Forwardness::Forward ? 1 : -1);
      }
      return result;
    }

    Iterator<IsForward>& operator--(int)
    {
      if (m_value == Invalid)
      {
        return *this;
      }
      else if (
        (IsForward == Forwardness::Forward && m_value == m_range[0]) ||
        (IsForward == Forwardness::Reverse && m_value == m_range[1] - 1))
      {
        m_value = Invalid;
      }
      else
      {
        m_value -= (IsForward == Forwardness::Forward ? 1 : -1);
      }
      return *this;
    }

    Iterator& operator+=(std::size_t count)
    {
      if (m_value == Invalid)
      {
        return *this;
      }
      else if (
        (IsForward == Forwardness::Forward && m_value + count >= m_range[1]) ||
        (IsForward == Forwardness::Reverse && m_value < m_range[0] + count))
      {
        m_value = Invalid;
      }
      else
      {
        // Disable a warning about subtraction and unsigned types from MSVC 2022;
        // we have already verified underflow will not occur.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4146)
#endif
        m_value += (IsForward == Forwardness::Forward ? count : -count);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
      }
      return *this;
    }

    Iterator& operator-=(std::size_t count)
    {
      if (m_value == Invalid)
      {
        return *this;
      }
      else if (
        (IsForward == Forwardness::Forward && m_value < m_range[0] + count) ||
        (IsForward == Forwardness::Reverse && m_value + count >= m_range[1]))
      {
        m_value = Invalid;
      }
      else
      {
        // Disable a warning about subtraction and unsigned types from MSVC 2022;
        // we have already verified underflow will not occur.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4146)
#endif
        m_value -= (IsForward == Forwardness::Forward ? count : -count);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
      }
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
    value_type index() const
    {
      if (m_value == Invalid)
      {
        return Invalid;
      }
      return m_value - m_range[0];
    }

    bool operator==(const Iterator& other) const
    {
      // NB: We could also test m_range == other.m_range and raise an exception.
      return m_value == other.m_value;
    }
    bool operator!=(const Iterator& other) const
    {
      // NB: We could also test m_range == other.m_range and raise an exception.
      return m_value != other.m_value;
    }

  protected:
    friend class SequentialAssignedIds;
    Iterator(IdType value, const IdRange& range)
      : m_value(value)
      , m_range(range)
    {
    }

    value_type m_value;
    IdRange m_range;
  };

  /// Returns the range of IDs in the allotment.
  IdRange range() const;
  /// Return the number of allotted IDs.
  /// This simply returns range()[1] - range()[0], since the range is a half-open interval.
  IdType size() const;
  /// Return the number of allotted IDs less 1 (as a convenience for iteration).
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
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_SequentialAssignedIds_h
