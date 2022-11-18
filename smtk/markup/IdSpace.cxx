//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/IdSpace.h"
#include "smtk/markup/AssignedIds.h"

template<typename T>
using discrete_interval = boost::icl::discrete_interval<T>;

namespace smtk
{
namespace markup
{

IdSpace::IdSpace(smtk::string::Token name)
  : Domain(name)
{
}

IdSpace::IdSpace(const nlohmann::json& data)
  : Domain(data)
{
  // TODO: Deserialize m_entries, m_range
}

const std::array<IdSpace::IdType, 2>& IdSpace::range() const
{
  return m_range;
}

std::array<IdSpace::IdType, 2>& IdSpace::range()
{
  return m_range;
}

shared_ptr<AssignedIds> IdSpace::defaultAssignment(
  const std::shared_ptr<IdSpace>& space,
  IdNature nature,
  IdType begin,
  IdType end)
{
  return std::make_shared<AssignedIds>(space, nature, begin, end, nullptr);
}

std::shared_ptr<AssignedIds> IdSpace::requestRange(
  IdNature nature,
  std::size_t rangeSize,
  std::size_t offset,
  std::function<
    std::shared_ptr<AssignedIds>(const std::shared_ptr<IdSpace>&, IdNature, IdType, IdType)> ctor)
{
  std::shared_ptr<AssignedIds> result;
  bool needToCheckRange = true;
  // If asked to choose an offset, do so.
  if (offset == Invalid)
  {
    switch (nature)
    {
      case IdNature::Referential:
        return result;
      default:
        // Choose an offset just beyond the largest ID we hold:
        // TODO: We could search for a hole to the left of or inside m_range.
        offset = (m_range[0] == Invalid ? 1 : m_range[1]);
        needToCheckRange = false;
        break;
    }
  }
  // Check that the range meets criteria imposed by \a nature.
  if (needToCheckRange)
  {
    bool rangeOK = true;
    switch (nature)
    {
      case IdNature::Primary:
        rangeOK = this->isRangeEmpty(offset, offset + rangeSize);
        break;
      case IdNature::NonExclusive:
      {
        IdType nn = this->numberOfIdsInRangeOfNature(offset, offset + rangeSize, IdNature::Primary);
        rangeOK = (nn == 0);
      }
      break;
      case IdNature::Referential:
      {
        IdType nn =
          this->numberOfIdsInRangeOfNature(offset, offset + rangeSize, IdNature::Unassigned);
        rangeOK = (nn == 0);
      }
      break;
      case IdNature::Unassigned:
        rangeOK = false;
        break;
    }
    if (!rangeOK)
    {
      return result;
    }
  }
  auto self = std::static_pointer_cast<IdSpace>(shared_from_this());
  result = ctor(self, nature, offset, offset + rangeSize);
  // Update m_range.
  if (m_range[0] == Invalid)
  {
    m_range = { offset, offset + rangeSize };
  }
  else
  {
    m_range[0] = offset < m_range[0] ? offset : m_range[0];
    m_range[1] = offset + rangeSize > m_range[1] ? offset + rangeSize : m_range[1];
  }
  m_entries += std::make_pair(
    discrete_interval<IdType>::right_open(result->range()[0], result->range()[1]),
    std::set<AssignedIds*>{ result.get() });
  return result;
}

void IdSpace::insertAssignment(const AssignedIds* assignedIds)
{
  if (!assignedIds || assignedIds->space().get() != this)
  {
    throw std::invalid_argument("Null assignment or mismatched ID space.");
  }
  // Update m_range.
  auto range = assignedIds->range();
  if (m_range[0] == Invalid)
  {
    m_range = { range[0], range[1] };
  }
  else
  {
    m_range[0] = range[0] < m_range[0] ? range[0] : m_range[0];
    m_range[1] = range[1] > m_range[1] ? range[1] : m_range[1];
  }
  // Add an interval-tree entry.
  m_entries += std::make_pair(
    discrete_interval<IdType>::right_open(assignedIds->range()[0], assignedIds->range()[1]),
    std::set<AssignedIds*>{ const_cast<AssignedIds*>(assignedIds) });
}

std::set<std::shared_ptr<AssignedIds>>
IdSpace::assignedIds(IdType begin, IdType end, IdNature nature) const
{
  std::set<std::shared_ptr<AssignedIds>> result;
  auto overlaps = m_entries & discrete_interval<IdType>::right_open(begin, end);
  for (const auto& overlap : overlaps)
  {
    for (const auto& assignment : overlap.second)
    {
      if (nature == Unassigned || nature == assignment->nature())
      {
        auto entry = assignment->shared_from_this();
        result.insert(entry);
      }
    }
  }
  return result;
}

bool IdSpace::isRangeEmpty(IdType begin, IdType end, IdNature nature) const
{
  auto overlaps = m_entries & discrete_interval<IdType>::right_open(begin, end);
  if (nature == IdNature::Unassigned)
  {
    return overlaps.empty();
  }
  std::size_t count = 0;
  for (const auto& overlap : overlaps)
  {
    for (const auto& assignment : overlap.second)
    {
      if (
        assignment->nature() == nature ||
        (nature == IdNature::Primary && assignment->nature() == IdNature::NonExclusive))
      {
        ++count;
      }
    }
  }
  return count == 0;
}

bool IdSpace::rangeHasPrimaryIds(IdType begin, IdType end) const
{
  IntervalMerge span;
  auto overlaps = m_entries & discrete_interval<IdType>::right_open(begin, end);
  std::set<AssignedIds*> visited;
  for (const auto& overlap : overlaps)
  {
    for (const auto& assignment : overlap.second)
    {
      if (assignment->nature() == IdNature::Primary)
      {
        if (visited.find(assignment) == visited.end())
        {
          visited.insert(assignment);
          span.insert(
            discrete_interval<IdType>::right_open(assignment->range()[0], assignment->range()[1]));
        }
      }
    }
  }
  bool hasPrimaryIds = false;
  for (const auto& interval : span)
  {
    if (interval.lower() <= begin && interval.upper() >= end)
    {
      hasPrimaryIds = true;
      break;
    }
  }
  std::cout << "Range [" << begin << " " << end << "[ " << (hasPrimaryIds ? "is" : "is not")
            << " covered by primaries.\n";
  return hasPrimaryIds;
}

IdSpace::IdType IdSpace::numberOfIdsInRangeOfNature(IdType begin, IdType end, IdNature nature) const
{
  // Because some ranges may have NonExclusive assignments (which unlike Primary assignments
  // may overlap one another), we implement this method using an interval_set() to combine
  // assignments clamped to [\a begin, \a end [. This collapses overlapping intervals
  IdType count = 0;
  IntervalMerge span;
  auto overlaps = m_entries & discrete_interval<IdType>::right_open(begin, end);
  for (const auto& overlap : overlaps)
  {
    for (const auto& assignment : overlap.second)
    {
      if (assignment->nature() == nature || nature == IdNature::Unassigned)
      {
        auto cr = IdSpace::clampedRange(assignment->range(), begin, end);
        span.insert(discrete_interval<IdType>::right_open(cr[0], cr[1]));
      }
    }
  }
  count = span.size();
  /* Just in case span.size() does funny stuff like omit holes, here is a sure way to compute count:
  count = 0;
  for (const auto& interval : span)
  {
    count += interval.upper() - interval.lower();
  }
  */

  // If we were asked to count unassigned IDs, take our count
  // of all assigned IDs and invert it:
  if (nature == IdNature::Unassigned)
  {
    count = end - begin - count;
  }
  return count;
}

bool IdSpace::removeEntry(const AssignedIds& entry)
{
  auto* entryPointer = const_cast<AssignedIds*>(&entry);
  auto span = std::make_pair(
    discrete_interval<IdType>::right_open(entry.range()[0], entry.range()[1]),
    std::set<AssignedIds*>{ entryPointer });
  m_entries -= span;
  return true;
}

std::array<IdSpace::IdType, 2>
IdSpace::clampedRange(const std::array<IdType, 2>& unclamped, IdType begin, IdType end)
{
  std::array<IdSpace::IdType, 2> result(unclamped);
  if (result[0] < begin)
  {
    result[0] = begin;
  }
  if (result[1] > end)
  {
    result[1] = end;
  }
  return result;
}

} // namespace markup
} // namespace smtk
