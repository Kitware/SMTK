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
#include "smtk/mesh/moab/IncrementalAllocator.h"
#include "smtk/mesh/moab/CellTypeToType.h"

#include "smtk/mesh/CellTypes.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/Interface.hpp"
#include "moab/ReadUtilIface.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <cassert>

namespace
{
  // must be a power of two
  static const std::size_t StartingAllocation = 64; // (1<<6)
}

namespace smtk {
namespace mesh {
namespace moab {

//----------------------------------------------------------------------------
IncrementalAllocator::IncrementalAllocator( ::moab::Interface* interface ):
  smtk::mesh::IncrementalAllocator(), BufferedCellAllocator(interface),
  m_index(0)
{
}

//----------------------------------------------------------------------------
void IncrementalAllocator::initialize()
{
  if (this->m_nCoordinates == 0)
    {
    this->IncrementalAllocator::allocateCoordinates(StartingAllocation);
    }
}

//----------------------------------------------------------------------------
bool IncrementalAllocator::allocateCoordinates(std::size_t nCoordinates)
{
  this->m_validState =
    this->BufferedCellAllocator::allocatePoints(nCoordinates,
                                                this->m_firstCoordinate,
                                                this->m_coordinateMemory);

  if (this->m_validState)
    {
    this->m_nCoordinates += nCoordinates;
    }

  this->m_coordinateMemories.push_back(this->m_coordinateMemory);

  return this->m_validState;
}

//----------------------------------------------------------------------------
std::size_t IncrementalAllocator::addCoordinate(double* xyz)
{
  if (!this->m_validState) { return false; }

  if (this->m_nCoordinates <= this->m_index)
    {
    this->IncrementalAllocator::allocateCoordinates(this->m_nCoordinates);
    if (!this->m_validState) { return this->m_index; }
    }

  this->m_validState =
    this->IncrementalAllocator::setCoordinate(this->m_index, xyz);

  return this->m_index++;
}

//----------------------------------------------------------------------------
bool IncrementalAllocator::setCoordinate(std::size_t coord, double* xyz)
{
  if (!this->m_validState) { return false; }

  if (coord >= this->m_nCoordinates)
    {
      return false;
    }

  // Coordinates are allocated using a memory doubling scheme, and we need to
  // figure out
  // (a) <exp>, the chunk of allocated memory in which <coord> resides, and
  // (b) <offset>, or starting index for chunck <exp>.
  // We do this by performing successive integer divisions by two (c >>= 1)
  // while keeping track of the number of divisions we have performed (++exp)
  // and by performing successive integer doublings of our offset by two
  // (offset <<= 1).

  std::size_t exp = 0;
  std::size_t offset = StartingAllocation >> 1;

  for (std::size_t c = coord; c >= StartingAllocation; c >>= 1)
  {
    ++exp;
    offset <<= 1;
  }

  if (exp > 0)
  {
    coord -= offset;
  }

  this->m_coordinateMemories[exp][0][coord] = xyz[0];
  this->m_coordinateMemories[exp][1][coord] = xyz[1];
  this->m_coordinateMemories[exp][2][coord] = xyz[2];

  return this->m_validState;
}

}
}
}
