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
  : m_rface(NULL)
{
  if (interface)
  {
    interface->query_interface(this->m_rface);
  }
}

Allocator::~Allocator()
{
  //don't de-allocate the Interface that created us, really manages this
  //memory
  this->m_rface = NULL;
}

bool Allocator::allocatePoints(std::size_t numPointsToAlloc, smtk::mesh::Handle& firstVertexHandle,
  std::vector<double*>& coordinateMemory)
{
  if (this->m_rface == NULL)
  {
    return false;
  }
  ::moab::ErrorCode err;
  err = this->m_rface->get_node_coords(3, //x,y,z
    static_cast<int>(numPointsToAlloc),
    0, //preferred_start_id
    firstVertexHandle, coordinateMemory);
  return err == ::moab::MB_SUCCESS;
}

bool Allocator::allocateCells(smtk::mesh::CellType cellType, std::size_t numCellsToAlloc,
  int numVertsPerCell, smtk::mesh::HandleRange& createdCellIds,
  smtk::mesh::Handle*& connectivityArray)
{
  if (this->m_rface == NULL)
  {
    return false;
  }
  ::moab::ErrorCode err;
  smtk::mesh::Handle startHandle;

  const int moabCellType = smtk::mesh::moab::smtkToMOABCell(cellType);

  err = this->m_rface->get_element_connect(static_cast<int>(numCellsToAlloc), numVertsPerCell,
    static_cast< ::moab::EntityType>(moabCellType),
    0, //preferred_start_id
    startHandle, connectivityArray);

  createdCellIds = smtk::mesh::HandleRange(startHandle, startHandle + numCellsToAlloc - 1);
  return err == ::moab::MB_SUCCESS;
}

bool Allocator::connectivityModified(const smtk::mesh::HandleRange& cellsToUpdate,
  int numVertsPerCell, const smtk::mesh::Handle* connectivityArray)
{
  if (this->m_rface == NULL)
  {
    return false;
  }

  const smtk::mesh::Handle& startHandle = cellsToUpdate.front();
  ::moab::ErrorCode err;
  err = this->m_rface->update_adjacencies(
    startHandle, static_cast<int>(cellsToUpdate.size()), numVertsPerCell, connectivityArray);
  return err == ::moab::MB_SUCCESS;
}
}
}
}
