//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_PointSet_h
#define __smtk_mesh_PointSet_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/Handle.h"
#include "smtk/mesh/QueryTypes.h"

namespace smtk {
namespace mesh {

class SMTKCORE_EXPORT PointSet
{
  friend PointSet set_intersect( const PointSet& a, const PointSet& b);
  friend PointSet set_difference( const PointSet& a, const PointSet& b);
  friend PointSet set_union( const PointSet& a, const PointSet& b );
  friend void for_each( const PointSet& a, PointForEach& filter);
public:
  PointSet(const smtk::mesh::CollectionPtr& parent,
           const smtk::mesh::HandleRange& points);

  //Copy Constructor required for rule of 3
  PointSet(const PointSet& other);

  //required to be in the cpp file as we hold a HandleRange
  ~PointSet();

  //Copy assignment operator required for rule of 3
  PointSet& operator= (const PointSet& other);
  bool operator==( const PointSet& other ) const;
  bool operator!=( const PointSet& other ) const;

  bool is_empty() const;
  std::size_t size() const;

  //Get the number of points in the array
  std::size_t numberOfPointSet() const;

  //returns true if the point set contains a given point id
  bool contains( const smtk::mesh::Handle& pointId ) const;

  //returns the index in this point set for the given point id
  std::size_t find( const smtk::mesh::Handle& pointId ) const;

  //Get all the point coordinates and store them in a pre-allocated buffer
  bool get(double* xyz) const;

  //Get all the point coordinates and store them in a pre-allocated buffer
  //Floats are not how we store the coordinates internally, so asking for
  //the coordinates in such a manner could cause data inaccuracies to appear
  //so generally this is only used if you fully understand the input domain
  bool get(float* xyz) const;

  //get the underlying HandleRange that this PointSet represents
  const smtk::mesh::HandleRange& range() const { return this->m_points; }

private:

  smtk::mesh::CollectionPtr m_parent;
  smtk::mesh::HandleRange m_points;
};

//intersect two set of points, placing the results in the return points object.
//This uses a point id based comparison, so points that have duplicate
//coordinates but different id's are considered to not intersect
//Note: If the points come from different collections the result will
//always be empty
SMTKCORE_EXPORT PointSet set_intersect( const PointSet& a, const PointSet& b);

//subtract points b from a, placing the results in the return points object.
//This uses a point id based comparison, so points that have duplicate
//coordinates but different id's are considered to not intersect
//Note: If the points come from different collections the result will
//always be empty
SMTKCORE_EXPORT PointSet set_difference( const PointSet& a, const PointSet& b);

//union two set of points, placing the results in the return points object.
//This uses a point id based comparison, so points that have duplicate
//coordinates but different id's are considered to not intersect
//Note: If the points come from different collections the result will
//always be empty
SMTKCORE_EXPORT PointSet set_union( const PointSet& a, const PointSet& b );

//apply a for_each point operator on each point in a container.
SMTKCORE_EXPORT void for_each( const PointSet& a, PointForEach& filter);

}
}

#endif
