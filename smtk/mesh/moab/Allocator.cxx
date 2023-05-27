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
#include "smtk/mesh/moab/Allocator.h"
#include "smtk/mesh/moab/CellTypeToType.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/Interface.hpp"
#include "moab/ReadUtilIface.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

namespace smtk
{
namespace mesh
{
namespace moab
{

Allocator::Allocator(::moab::Interface* interface)
{
  if (interface)
  {
    interface->query_interface(m_rface);
  }
}

Allocator::~Allocator()
{
  //don't de-allocate the Interface that created us, really manages this
  //memory
  m_rface = nullptr;
}

bool Allocator::allocatePoints(
  std::size_t numPointsToAlloc,
  smtk::mesh::Handle& firstVertexHandle,
  std::vector<double*>& coordinateMemory)
{
  if (m_rface == nullptr)
  {
    return false;
  }
  ::moab::ErrorCode err;
  err = m_rface->get_node_coords(
    3, //x,y,z
    static_cast<int>(numPointsToAlloc),
    0, //preferred_start_id
    firstVertexHandle,
    coordinateMemory);
  return err == ::moab::MB_SUCCESS;
}

bool Allocator::allocateCells(
  smtk::mesh::CellType cellType,
  std::size_t numCellsToAlloc,
  int numVertsPerCell,
  smtk::mesh::HandleRange& createdCellIds,
  smtk::mesh::Handle*& connectivityArray)
{
  if (m_rface == nullptr)
  {
    return false;
  }
  ::moab::ErrorCode err;
  smtk::mesh::Handle startHandle;

  const int moabCellType = smtk::mesh::moab::smtkToMOABCell(cellType);

  err = m_rface->get_element_connect(
    static_cast<int>(numCellsToAlloc),
    numVertsPerCell,
    static_cast<::moab::EntityType>(moabCellType),
    0, //preferred_start_id
    startHandle,
    connectivityArray);

  createdCellIds = smtk::mesh::HandleRange(
    smtk::mesh::HandleInterval(startHandle, startHandle + numCellsToAlloc - 1));
  return err == ::moab::MB_SUCCESS;
}

bool Allocator::connectivityModified(
  const smtk::mesh::HandleRange& cellsToUpdate,
  int numVertsPerCell,
  const smtk::mesh::Handle* connectivityArray)
{
  return this->connectivityModified(
    smtk::mesh::rangeElement(cellsToUpdate, 0),
    static_cast<int>(cellsToUpdate.size()),
    numVertsPerCell,
    connectivityArray);
}

bool Allocator::connectivityModified(
  smtk::mesh::Handle firstCellToUpdate,
  int numberOfCellsToUpdate,
  int numVertsPerCell,
  const smtk::mesh::Handle* connectivityArray)
{
  if (m_rface == nullptr)
  {
    return false;
  }

  ::moab::ErrorCode err;
  err = m_rface->update_adjacencies(
    firstCellToUpdate, numberOfCellsToUpdate, numVertsPerCell, connectivityArray);
  return err == ::moab::MB_SUCCESS;
}
} // namespace moab
} // namespace mesh
} // namespace smtk
