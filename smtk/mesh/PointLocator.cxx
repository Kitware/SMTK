//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/PointLocator.h"
#include "smtk/mesh/Collection.h"

namespace smtk {
namespace mesh {

//----------------------------------------------------------------------------
PointLocator::PointLocator(const smtk::mesh::PointSet& ps):
  m_locator(ps.collection()->interface()->pointLocator( ps.range() ) )
{

}

//----------------------------------------------------------------------------
PointLocator::PointLocator(const smtk::mesh::CollectionPtr collection,
                           const double* const xyzs,
                           std::size_t numPoints):
  m_locator(collection->interface()->pointLocator(xyzs, numPoints) )
{

}

//----------------------------------------------------------------------------
PointLocator::PointLocator(const smtk::mesh::CollectionPtr collection,
                           const float* const xyzs,
                           std::size_t numPoints):
  m_locator(collection->interface()->pointLocator(xyzs, numPoints) )
{

}

//----------------------------------------------------------------------------
void PointLocator::find(double x, double y, double z, double radius,
                        LocatorResults& results)
{
  return this->m_locator->locatePointsWithinRadius(x,y,z,radius,results);
}

}
}
