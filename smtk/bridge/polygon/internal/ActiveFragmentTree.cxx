//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/polygon/internal/ActiveFragmentTree.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

ActiveFragmentTree::ActiveFragmentTree(FragmentArray& fragments, SweeplinePosition& posn)
  :
    ActiveFragmentTreeType(
      EdgeFragmentComparator(fragments, posn))
{
}

void ActiveFragmentTree::insertActiveFragment(FragmentId f)
{
  this->insert(f);
}

std::pair<FragmentId, FragmentId> ActiveFragmentTree::boundingFragments(const internal::Point& pt, bool strict) const
{
  std::pair<FragmentId, FragmentId> result(-1, -1);
  ActiveFragmentTreeType::const_iterator it;
  bool onLower = false;
  for (it = this->begin(); it != this->end(); ++it)
    {
    const EdgeFragment& frag(this->fragment(*it));
    internal::HighPrecisionCoord dlx =
      static_cast<smtk::bridge::polygon::internal::HighPrecisionCoord>(frag.hi().x() - frag.lo().x());
    internal::HighPrecisionCoord dly =
      static_cast<smtk::bridge::polygon::internal::HighPrecisionCoord>(frag.hi().y() - frag.lo().y());

    internal::HighPrecisionCoord dst = dly * pt.x() - dlx * pt.y() - dly * frag.lo().x() + dlx * frag.lo().y();
    if (dst == 0 && !strict)
      {
      // Only record this fragment as a lower bound if there are no strict bounds already
      if (!onLower && result.first == FragmentId(-1))
        {
        result.first = *it;
        }
      onLower = true;
      if (result.second == FragmentId(-1))
        {
        result.second = *it;
        }
      // Only record this fragment as an upper bound if there are no strict bounds (i.e., do not terminate the search).
      continue;
      }
    else if (dst < 0 && !onLower)
      {
      result.first = *it;
      }
    else if (dst > 0)
      {
      result.second = *it;
      break;
      }
    }
  return result;
}

const EdgeFragment& ActiveFragmentTree::fragment(FragmentId f) const
{
  return (*this->key_comp().fragments())[f];
}

    } // namespace polygon
  } //namespace bridge
} // namespace smtk
