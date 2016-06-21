//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_PointLocator_h
#define __smtk_mesh_PointLocator_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/Interface.h"
#include "smtk/mesh/PointSet.h"

namespace smtk {
namespace mesh {

//PointLocator facilitates searching for points in 3D.
//Currently the class has a fairly narrow API as we are still iterating
//what features are required by a smtk::mesh::PointLocator
//
//
//
class SMTKCORE_EXPORT PointLocator
{
public:
  typedef smtk::mesh::PointLocatorImpl::Results LocatorResults;

  //Construct a point locator given an existing set of points
  //These are the points you will be searching against
  PointLocator(const smtk::mesh::PointSet& ps);

  //Construct a point locator given a set of raw points.
  //Based on the backend these points maybe be added to the collection for
  //duration of the PointLocator
  //Set ignoreZValues to true if you want all the points to be located
  //on a plane at Z == 0.0
  PointLocator(const smtk::mesh::CollectionPtr collection,
               const double* const xyzs,
               std::size_t numPoints,
               bool ignoreZValues=false);

  //Construct a point locator given a set of raw points.
  //Based on the backend these points maybe be added to the collection for
  //duration of the PointLocator
  //Set ignoreZValues to true if you want all the points to be located
  //on a plane at Z == 0.0
  PointLocator(const smtk::mesh::CollectionPtr collection,
               const float* const xyzs,
               std::size_t numPoints,
               bool ignoreZValues=false);

  //returns all the point ids that are inside the locator
  smtk::mesh::HandleRange range() const;

  //Find the set of points that are within the radius of a single point.
  //
  //See smtk/mesh/Interface.h for the full implementation of Results
  //but the basics are:
  //
  //
  //struct Results
  //  {
  //  smtk::mesh::HandleRange pointIds;
  //  std::vector<double> sqDistances;
  //  std::vector<double> x_s, y_s, z_s;
  //  bool want_sqDistances;
  //  bool want_Coordinates;
  //  };
  //
  void find(double x, double y, double z, double radius, LocatorResults& results);

private:
  smtk::mesh::PointLocatorImplPtr m_locator;
};

}
}

#endif
