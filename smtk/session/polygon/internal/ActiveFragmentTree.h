//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef smtk_session_polygon_ActiveFragmentTree_h
#define smtk_session_polygon_ActiveFragmentTree_h

#include "smtk/session/polygon/internal/Fragment.h" // for various internal types help by CreateFaces

namespace smtk
{
namespace session
{
namespace polygon
{

typedef std::set<FragmentId, EdgeFragmentComparator> ActiveFragmentTreeType;

/// The sweepline Interval Tree (IT), of active edge segments, is a set of offsets into the array of fragments.
class ActiveFragmentTree : public ActiveFragmentTreeType
{
public:
  ActiveFragmentTree(FragmentArray& fragments, SweeplinePosition& posn);

  using ActiveFragmentTreeType::begin;
  using ActiveFragmentTreeType::end;
  using ActiveFragmentTreeType::erase;
  using ActiveFragmentTreeType::insert;
  using ActiveFragmentTreeType::lower_bound;
  using ActiveFragmentTreeType::rbegin;
  using ActiveFragmentTreeType::rend;
  using ActiveFragmentTreeType::upper_bound;

  void insertActiveFragment(FragmentId f);
  std::pair<FragmentId, FragmentId> boundingFragments(
    const internal::Point& pt,
    bool strict = false) const;

  const EdgeFragment& fragment(FragmentId f) const;
};

} // namespace polygon
} //namespace session
} // namespace smtk

#endif // smtk_session_polygon_ActiveFragmentTree_h
