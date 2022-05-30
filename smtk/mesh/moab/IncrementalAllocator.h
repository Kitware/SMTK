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
#ifndef smtk_mesh_moab_IncrementalAllocator_h
#define smtk_mesh_moab_IncrementalAllocator_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/moab/BufferedCellAllocator.h"
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

class SMTKCORE_EXPORT IncrementalAllocator
  : public smtk::mesh::IncrementalAllocator
  , protected smtk::mesh::moab::BufferedCellAllocator
{
public:
  IncrementalAllocator(::moab::Interface* interface);

  ~IncrementalAllocator() override = default;

  IncrementalAllocator(const IncrementalAllocator& other) = delete;
  IncrementalAllocator& operator=(const IncrementalAllocator& other) = delete;

  std::size_t addCoordinate(double* xyz) override;
  bool setCoordinate(std::size_t coord, double* xyz) override;

  bool addCell(smtk::mesh::CellType ctype, long long int* pointIds, std::size_t nCoordinates = 0)
    override
  {
    return BufferedCellAllocator::addCell(ctype, pointIds, nCoordinates);
  }
  bool addCell(smtk::mesh::CellType ctype, long int* pointIds, std::size_t nCoordinates = 0)
    override
  {
    return BufferedCellAllocator::addCell(ctype, pointIds, nCoordinates);
  }
  bool addCell(smtk::mesh::CellType ctype, int* pointIds, std::size_t nCoordinates = 0) override
  {
    return BufferedCellAllocator::addCell(ctype, pointIds, nCoordinates);
  }

  bool flush() override { return BufferedCellAllocator::flush(); }

  smtk::mesh::HandleRange cells() override { return BufferedCellAllocator::cells(); }

  bool isValid() const override { return BufferedCellAllocator::isValid(); }

protected:
  bool allocateCoordinates(std::size_t nCoordinates);

  friend class Interface;
  void initialize();

private:
  std::size_t m_index{ 0 };
  std::vector<std::vector<double*>> m_coordinateMemories;
};
} // namespace moab
} // namespace mesh
} // namespace smtk

#endif
