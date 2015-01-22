//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_PointConnectivity_h
#define __smtk_mesh_PointConnectivity_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/Handle.h"

namespace smtk {
namespace mesh {

//PointConnectivity is the mapping of a cell's id to a set of point ids that
//represent that cells physical location. We need to answer how in the long
//run we are going to be able to get exact locations of the point ids
//that PointConnectivity returns
class SMTKCORE_EXPORT PointConnectivity
{
  //in the future I am expecting we will need a custom iterator
  //time to dig into boost::iterator_facade.
  // typedef ContainerType::const_iterator const_iterator;
  // typedef ContainerType::iterator iterator;
public:
  typedef smtk::mesh::Handle value_type;
  typedef value_type& reference;
  typedef const value_type& const_reference;

  PointConnectivity(const smtk::mesh::CollectionPtr& parent,
               const smtk::mesh::HandleRange& range);

  PointConnectivity(const smtk::mesh::CollectionPtr& parent,
               const smtk::mesh::Handle& cell);

  //Copy Constructor required for rule of 3
  PointConnectivity(const PointConnectivity& other);

  //required to be in the cpp file as we hold a HandleRange
  ~PointConnectivity();

  //Copy assignment operator required for rule of 3
  PointConnectivity& operator= (const PointConnectivity& other);
  bool operator==( const PointConnectivity& other ) const;
  bool operator!=( const PointConnectivity& other ) const;

  //Get the total number of vertices's in the connectivity
  std::size_t size() const;

  //Get the number of cells in the array
  std::size_t numberOfCells() const;

  bool is_empty() const;

  //start cell traversal of the vertices
  void initCellTraversal();

  //fetch the handles to the points of the cell.
  //The pointer that is returned must not be deleted.
  //The pointer returned should be treated as a read only pointer
  bool fetchNextCell(int& numPts, const smtk::mesh::Handle* &points);

  //struct that holds the required information to compute what is the
  //current cell when we are iterating.
  struct IterationState
    {
    IterationState():
      whichConnectivityVector(0),
      ptrOffsetInVector(0)
      {
      }

    std::size_t whichConnectivityVector;
    std::size_t ptrOffsetInVector;
    };
private:

  smtk::mesh::CollectionPtr m_parent;

  class InternalStorageImpl;
  smtk::shared_ptr< InternalStorageImpl > m_connectivity;



  IterationState m_iteratorLocation;
};

}
}

#endif
