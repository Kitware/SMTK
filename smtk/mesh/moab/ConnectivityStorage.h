//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_mesh_moab_ConnectivityStorage_h
#define smtk_mesh_moab_ConnectivityStorage_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/mesh/core/Handle.h"

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

  ~ConnectivityStorage() override;

  ConnectivityStorage(const ConnectivityStorage& other) = delete;
  ConnectivityStorage& operator=(const ConnectivityStorage& other) = delete;

  void initTraversal(smtk::mesh::ConnectivityStorage::IterationState& state) override;

  bool fetchNextCell(
    smtk::mesh::ConnectivityStorage::IterationState& state,
    smtk::mesh::CellType& cellType,
    int& numPts,
    const smtk::mesh::Handle*& points) override;

  bool equal(smtk::mesh::ConnectivityStorage* other) const override;

  std::size_t cellSize() const override { return NumberOfCells; }

  std::size_t vertSize() const override { return NumberOfVerts; }

private:
  std::vector<const smtk::mesh::Handle*> ConnectivityStartPositions;
  std::vector<int> ConnectivityArraysLengths;
  std::vector<int> ConnectivityVertsPerCell;
  std::vector<smtk::mesh::CellType> ConnectivityTypePerCell;
  std::size_t NumberOfCells{ 0 };
  std::size_t NumberOfVerts{ 0 };

  //moab vertices don't have connectivity so we create our own
  std::vector<smtk::mesh::Handle> VertConnectivityStorage;
};
} // namespace moab
} // namespace mesh
} // namespace smtk

#endif
