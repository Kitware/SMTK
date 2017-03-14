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
#ifndef __smtk_mesh_moab_IncrementalAllocator_h
#define __smtk_mesh_moab_IncrementalAllocator_h

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

namespace smtk {
namespace mesh {
namespace moab {

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT IncrementalAllocator :
    public smtk::mesh::IncrementalAllocator,
    protected smtk::mesh::moab::BufferedCellAllocator
{
public:
  IncrementalAllocator( ::moab::Interface* interface );

  virtual ~IncrementalAllocator() {}

  std::size_t addCoordinate(double* xyz);
  bool setCoordinate(std::size_t coord, double* xyz);

  bool addCell(smtk::mesh::CellType ctype, long long int* pointIds,
               std::size_t nCoordinates = 0)
  { return BufferedCellAllocator::addCell(ctype, pointIds, nCoordinates); }
  virtual bool addCell(smtk::mesh::CellType ctype, long int* pointIds,
                       std::size_t nCoordinates = 0)
  { return BufferedCellAllocator::addCell(ctype, pointIds, nCoordinates); }
  virtual bool addCell(smtk::mesh::CellType ctype, int* pointIds,
                       std::size_t nCoordinates = 0)
  { return BufferedCellAllocator::addCell(ctype, pointIds, nCoordinates); }

  virtual bool flush() { return BufferedCellAllocator::flush(); }

  virtual smtk::mesh::HandleRange cells()
  { return BufferedCellAllocator::cells(); }

  virtual bool isValid() const { return BufferedCellAllocator::isValid(); }

protected:
  bool allocateCoordinates(std::size_t nCoordinates);

  friend class Interface;
  void initialize();

private:
  IncrementalAllocator( const IncrementalAllocator& other ); //blank since we are used by shared_ptr
  IncrementalAllocator& operator=( const IncrementalAllocator& other ); //blank since we are used by shared_ptr

  std::size_t m_index;
  std::vector<std::vector<double*> > m_coordinateMemories;
};

}
}
}

#endif
