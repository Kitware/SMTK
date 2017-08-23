//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_moab_PointLocatorImpl_h
#define __smtk_mesh_moab_PointLocatorImpl_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/moab/Interface.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/AdaptiveKDTree.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

namespace smtk
{
namespace mesh
{
namespace moab
{

class SMTKCORE_EXPORT PointLocatorImpl : public smtk::mesh::PointLocatorImpl
{
public:
  PointLocatorImpl(::moab::Interface* interface, const smtk::mesh::HandleRange& points);

  PointLocatorImpl(::moab::Interface* interface, std::size_t numPoints,
    const std::function<std::array<double, 3>(std::size_t)>& coordinates);

  ~PointLocatorImpl();

  smtk::mesh::HandleRange range() const override;

  //returns the set of points that are within the radius of a single point
  void locatePointsWithinRadius(
    double x, double y, double z, double radius, Results& results) override;

private:
  ::moab::Interface* m_interface;
  smtk::mesh::Handle m_meshOwningPoints;
  bool m_deletePoints;
  ::moab::AdaptiveKDTree m_tree;
};
}
}
}

#endif
