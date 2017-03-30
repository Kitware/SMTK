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
#include "smtk/mesh/moab/BufferedCellAllocator.h"
#include "smtk/mesh/moab/CellTypeToType.h"

#include "smtk/mesh/CellTypes.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/Interface.hpp"
#include "moab/ReadUtilIface.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <cassert>

namespace smtk
{
namespace mesh
{
namespace moab
{

BufferedCellAllocator::BufferedCellAllocator(::moab::Interface* interface)
  : smtk::mesh::BufferedCellAllocator()
  , Allocator(interface)
  , m_firstCoordinate(0)
  , m_nCoordinates(0)
  , m_coordinateMemory()
  , m_activeCellType(smtk::mesh::CellType_MAX)
  , m_nCoords(0)
  , m_localConnectivity()
  , m_cells()
{
}

BufferedCellAllocator::~BufferedCellAllocator()
{
  this->flush();
}

bool BufferedCellAllocator::reserveNumberOfCoordinates(std::size_t nCoordinates)
{
  // Can only reserve coordinates once
  if (this->m_nCoordinates != 0)
  {
    return false;
  }

  this->m_validState =
    this->allocatePoints(nCoordinates, this->m_firstCoordinate, this->m_coordinateMemory);

  if (this->m_validState)
  {
    this->m_nCoordinates = nCoordinates;
  }

  return this->m_validState;
}

bool BufferedCellAllocator::setCoordinate(std::size_t coord, double* xyz)
{
  if (!this->m_validState)
  {
    return false;
  }
  assert(coord < this->m_nCoordinates);

  this->m_coordinateMemory[0][coord] = xyz[0];
  this->m_coordinateMemory[1][coord] = xyz[1];
  this->m_coordinateMemory[2][coord] = xyz[2];

  return this->m_validState;
}

bool BufferedCellAllocator::flush()
{
  if (!this->m_validState)
  {
    return false;
  }

  if (this->m_localConnectivity.empty())
  {
    return true;
  }

  if (this->m_activeCellType == smtk::mesh::CellType_MAX)
  {
    return false;
  }

  if (this->m_activeCellType == smtk::mesh::Vertex)
  {
    // In the moab/interface world vertices don't have explicit connectivity
    // so we can't allocate cells. Instead we just explicitly add those
    // points to the cells range
    for (auto&& ptCoordinate : this->m_localConnectivity)
    {
      this->m_cells.insert(this->m_firstCoordinate + ptCoordinate);
    }

    this->m_localConnectivity.clear();

    return this->m_validState;
  }

  // only convert cells smtk mesh supports
  ::moab::Range cellsCreatedForThisType;

  // need to convert from smtk cell type to moab cell type
  ::moab::EntityHandle* startOfConnectivityArray = 0;

  this->m_validState =
    this->allocateCells(this->m_activeCellType, this->m_localConnectivity.size() / this->m_nCoords,
      this->m_nCoords, cellsCreatedForThisType, startOfConnectivityArray);

  if (this->m_validState)
  {
    // now that we have the chunk allocated need to fill it
    // we do this by iterating the cells
    for (std::size_t i = 0; i < this->m_localConnectivity.size(); ++i)
    {
      startOfConnectivityArray[i] = this->m_firstCoordinate + this->m_localConnectivity[i];
    }

    // notify database that we have written to connectivity, that way
    // it can properly update adjacencies and other database info
    this->connectivityModified(this->m_cells, this->m_nCoords, startOfConnectivityArray);

    // insert these cells back into the range
    this->m_cells.insert(cellsCreatedForThisType.begin(), cellsCreatedForThisType.end());
  }

  this->m_localConnectivity.clear();

  return this->m_validState;
}
}
}
}
