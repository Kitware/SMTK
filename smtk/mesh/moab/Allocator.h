//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#ifndef smtk_mesh_moab_Allocator_h
#define smtk_mesh_moab_Allocator_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/moab/Interface.h"

namespace moab
{
class ReadUtilIface;
}

namespace smtk
{
namespace mesh
{
namespace moab
{

class SMTKCORE_EXPORT Allocator : public smtk::mesh::Allocator
{
public:
  Allocator(::moab::Interface* interface);

  ~Allocator() override;

  Allocator(const Allocator& other) = delete;
  Allocator& operator=(const Allocator& other) = delete;

  bool allocatePoints(
    std::size_t numPointsToAlloc,
    smtk::mesh::Handle& firstVertexHandle,
    std::vector<double*>& coordinateMemory) override;

  bool allocateCells(
    smtk::mesh::CellType cellType,
    std::size_t numCellsToAlloc,
    int numVertsPerCell,
    smtk::mesh::HandleRange& createdCellIds,
    smtk::mesh::Handle*& connectivityArray) override;

  bool connectivityModified(
    const smtk::mesh::HandleRange& cellsToUpdate,
    int numVertsPerCell,
    const smtk::mesh::Handle* connectivityArray) override;

protected:
  bool connectivityModified(
    smtk::mesh::Handle firstCellToUpdate,
    int numberOfCellsToUpdate,
    int numVertsPerCell,
    const smtk::mesh::Handle* connectivityArray);

private:
  //holds a reference to the real moab interface
  ::moab::ReadUtilIface* m_rface = nullptr;
};
} // namespace moab
} // namespace mesh
} // namespace smtk

#endif
