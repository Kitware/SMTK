//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_polygon_internal_Region_h
#define __smtk_session_polygon_internal_Region_h

#include "smtk/bridge/polygon/Operator.h"
#include "smtk/bridge/polygon/internal/Fragment.h"

#include "smtk/common/UnionFind.h"

#include <map>
#include <set>
#include <vector>

namespace smtk
{
namespace bridge
{
namespace polygon
{

/// The set of all regions is a UnionFind (UF) data structure.
typedef smtk::common::UnionFind<int> RegionIdSet;

/**\brief A structure to hold chains of coedges bounding regions of space.
  *
  */
class SMTKPOLYGONSESSION_EXPORT Region
{
public:
  FragmentId m_seedFragment;
  bool m_seedSense;
  //std::deque<std::pair<FragmentId,bool> > m_boundary; // size_t = fragment id, bool = sense rel to fragment
  std::set<int> m_innerLoops;

  Region();
  Region(FragmentId seedFrag, bool seedSense);

  void merge(const Region* other);
};

/// A map to hold each region's definition indexed by its UF region ID.
typedef std::map<RegionIdSet::value_type, smtk::shared_ptr<Region> > RegionDefinitions;

} // namespace polygon
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_internal_Region_h
