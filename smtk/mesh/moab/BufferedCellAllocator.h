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
#ifndef smtk_mesh_moab_BufferedCellAllocator_h
#define smtk_mesh_moab_BufferedCellAllocator_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/moab/Allocator.h"
#include "smtk/mesh/moab/Interface.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/EntityHandle.hpp"
#include "moab/Range.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <cassert>
#include <cstdint>

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

class SMTKCORE_EXPORT BufferedCellAllocator
  : public smtk::mesh::BufferedCellAllocator
  , protected smtk::mesh::moab::Allocator
{
public:
  BufferedCellAllocator(::moab::Interface* interface);

  ~BufferedCellAllocator() override;

  BufferedCellAllocator(const BufferedCellAllocator& other) = delete;
  BufferedCellAllocator& operator=(const BufferedCellAllocator& other) = delete;

  bool reserveNumberOfCoordinates(std::size_t nCoordinates) override;
  bool setCoordinate(std::size_t coord, double* xyz) override;

  bool addCell(smtk::mesh::CellType ctype, long long int* pointIds, std::size_t nCoordinates = 0)
    override
  {
    return this->addCell<long long int>(ctype, pointIds, nCoordinates);
  }
  bool addCell(smtk::mesh::CellType ctype, long int* pointIds, std::size_t nCoordinates = 0)
    override
  {
    return this->addCell<long int>(ctype, pointIds, nCoordinates);
  }
  bool addCell(smtk::mesh::CellType ctype, int* pointIds, std::size_t nCoordinates = 0) override
  {
    return this->addCell<int>(ctype, pointIds, nCoordinates);
  }

  bool flush() override;

  smtk::mesh::HandleRange cells() override;

  void clear();

protected:
  template<typename IntegerType>
  bool addCell(smtk::mesh::CellType ctype, IntegerType* pointIds, std::int64_t nCoordinates);

  ::moab::EntityHandle m_firstCoordinate{ 0 };
  std::size_t m_nCoordinates{ 0 };
  std::vector<double*> m_coordinateMemory;
  smtk::mesh::CellType m_activeCellType{ smtk::mesh::CellType_MAX };
  int m_nCoords{ 0 };
  std::vector<std::int64_t> m_localConnectivity;
  ::moab::Range m_cells;
};

template<typename IntegerType>
bool BufferedCellAllocator::addCell(
  smtk::mesh::CellType ctype,
  IntegerType* pointIds,
  std::int64_t nCoordinates)
{
  if (!m_validState)
  {
    return false;
  }

  if (ctype != m_activeCellType || (nCoordinates != 0 && nCoordinates != m_nCoords))
  {
    m_validState = this->flush();
    m_activeCellType = ctype;
    m_nCoords =
      nCoordinates != 0 ? static_cast<int>(nCoordinates) : smtk::mesh::verticesPerCell(ctype);
  }

  assert(m_activeCellType != smtk::mesh::CellType_MAX);
  assert(m_nCoords > 0);

  for (std::int64_t i = 0; i < m_nCoords; i++)
  {
    m_localConnectivity.push_back(pointIds[i]);
  }

  return m_validState;
}
} // namespace moab
} // namespace mesh
} // namespace smtk

#endif
