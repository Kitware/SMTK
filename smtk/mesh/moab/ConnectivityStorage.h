//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_moab_ConnectivityStorage_h
#define __smtk_mesh_moab_ConnectivityStorage_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/mesh/Handle.h"

#include "smtk/mesh/moab/Interface.h"

namespace smtk
{
namespace mesh
{
namespace moab
{

class SMTKCORE_EXPORT ConnectivityStorage : public smtk::mesh::ConnectivityStorage
{
public:
  ConnectivityStorage(::moab::Interface* interface, const smtk::mesh::HandleRange& cells);

  virtual ~ConnectivityStorage();

  void initTraversal(smtk::mesh::ConnectivityStorage::IterationState& state);

  bool fetchNextCell(smtk::mesh::ConnectivityStorage::IterationState& state,
    smtk::mesh::CellType& cellType, int& numPts, const smtk::mesh::Handle*& points);

  bool equal(smtk::mesh::ConnectivityStorage* other) const;

  std::size_t cellSize() const { return NumberOfCells; }

  std::size_t vertSize() const { return NumberOfVerts; }

private:
  //blank since we are used by shared_ptr
  ConnectivityStorage(const ConnectivityStorage& other);
  //blank since we are used by shared_ptr
  ConnectivityStorage& operator=(const ConnectivityStorage& other);

  std::vector<const smtk::mesh::Handle*> ConnectivityStartPositions;
  std::vector<int> ConnectivityArraysLengths;
  std::vector<int> ConnectivityVertsPerCell;
  std::vector<smtk::mesh::CellType> ConnectivityTypePerCell;
  std::size_t NumberOfCells;
  std::size_t NumberOfVerts;

  //moab vertices don't have connectivity so we create our own
  std::vector<smtk::mesh::Handle> VertConnectivityStorage;
};
}
}
}

#endif
