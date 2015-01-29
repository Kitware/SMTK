//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_moab_PointConnectivityStorage_h
#define __smtk_mesh_moab_PointConnectivityStorage_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/mesh/Handle.h"
#include "smtk/mesh/PointConnectivity.h"

namespace smtk {
namespace mesh {
namespace moab {

class PointConnectivityStorage
{
public:

  PointConnectivityStorage(const smtk::mesh::InterfacePtr& iface,
                           const smtk::mesh::HandleRange& cells);

  PointConnectivityStorage(const smtk::mesh::InterfacePtr& iface,
                           const smtk::mesh::Handle& cell);


  void initTraversal( smtk::mesh::PointConnectivity::IterationState& state );

  bool fetchNextCell( smtk::mesh::PointConnectivity::IterationState& state,
                      int& numPts,
                      const smtk::mesh::Handle* &points);

  bool equal( PointConnectivityStorage* other ) const;

  std::size_t cellSize() const { return NumberOfCells; }

  std::size_t vertSize() const { return NumberOfVerts; }

private:
  std::vector< const smtk::mesh::Handle* > ConnectivityStartPositions;
  std::vector<int> ConnectivityArraysLengths;
  std::vector<int> ConnectivityVertsPerCell;
  std::size_t NumberOfCells;
  std::size_t NumberOfVerts;
};

}
}
}

#endif
