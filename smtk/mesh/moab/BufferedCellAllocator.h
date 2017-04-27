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
#ifndef __smtk_mesh_moab_BufferedCellAllocator_h
#define __smtk_mesh_moab_BufferedCellAllocator_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/moab/Allocator.h"
#include "smtk/mesh/moab/Interface.h"

#include <cstdint>

#include <cassert>

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

class SMTKCORE_EXPORT BufferedCellAllocator : public smtk::mesh::BufferedCellAllocator,
                                              protected smtk::mesh::moab::Allocator
{
public:
  BufferedCellAllocator(::moab::Interface* interface);

  virtual ~BufferedCellAllocator();

  bool reserveNumberOfCoordinates(std::size_t nCoordinates);
  bool setCoordinate(std::size_t coord, double* xyz);

  bool addCell(smtk::mesh::CellType ctype, long long int* pointIds, std::size_t nCoordinates = 0)
  {
    return this->addCell<long long int>(ctype, pointIds, nCoordinates);
  }
  bool addCell(smtk::mesh::CellType ctype, long int* pointIds, std::size_t nCoordinates = 0)
  {
    return this->addCell<long int>(ctype, pointIds, nCoordinates);
  }
  bool addCell(smtk::mesh::CellType ctype, int* pointIds, std::size_t nCoordinates = 0)
  {
    return this->addCell<int>(ctype, pointIds, nCoordinates);
  }

  bool flush();

  smtk::mesh::HandleRange cells() { return this->m_cells; }

protected:
  template <typename IntegerType>
  bool addCell(smtk::mesh::CellType ctype, IntegerType* pointIds, std::int64_t nCoordinates);

  ::moab::EntityHandle m_firstCoordinate;
  std::size_t m_nCoordinates;
  std::vector<double*> m_coordinateMemory;
  smtk::mesh::CellType m_activeCellType;
  int m_nCoords;
  std::vector<std::int64_t> m_localConnectivity;
  ::moab::Range m_cells;

private:
  BufferedCellAllocator(const BufferedCellAllocator& other); //blank since we are used by shared_ptr
  BufferedCellAllocator& operator=(
    const BufferedCellAllocator& other); //blank since we are used by shared_ptr
};

template <typename IntegerType>
bool BufferedCellAllocator::addCell(
  smtk::mesh::CellType ctype, IntegerType* pointIds, std::int64_t nCoordinates)
{
  if (!this->m_validState)
  {
    return false;
  }

  if (ctype != this->m_activeCellType ||
    (ctype == smtk::mesh::Polygon && nCoordinates != this->m_nCoords))
  {
    this->m_validState = this->flush();
    this->m_activeCellType = ctype;
    this->m_nCoords = ctype != smtk::mesh::Polygon ? smtk::mesh::verticesPerCell(ctype)
                                                   : static_cast<int>(nCoordinates);
  }

  assert(this->m_activeCellType != smtk::mesh::CellType_MAX);
  assert(this->m_nCoords > 0);

  for (std::int64_t i = 0; i < this->m_nCoords; i++)
  {
    this->m_localConnectivity.push_back(pointIds[i]);
  }

  return this->m_validState;
}
}
}
}

#endif
