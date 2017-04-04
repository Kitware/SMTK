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
#ifndef __smtk_mesh_moab_Allocator_h
#define __smtk_mesh_moab_Allocator_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/moab/Interface.h"

namespace moab
{
  class ReadUtilIface;
}

namespace smtk {
namespace mesh {
namespace moab {

class SMTKCORE_EXPORT Allocator : public smtk::mesh::Allocator
{
public:
  Allocator( ::moab::Interface* interface );

  virtual ~Allocator();

  bool allocatePoints( std::size_t numPointsToAlloc,
                       smtk::mesh::Handle& firstVertexHandle,
                       std::vector<double* >& coordinateMemory);

  bool allocateCells( smtk::mesh::CellType cellType,
                      std::size_t numCellsToAlloc,
                      int numVertsPerCell,
                      smtk::mesh::HandleRange& createdCellIds,
                      smtk::mesh::Handle*& connectivityArray);

  bool connectivityModified( const smtk::mesh::HandleRange& cellsToUpdate,
                             int numVertsPerCell,
                             const smtk::mesh::Handle* connectivityArray);
private:
  Allocator( const Allocator& other ); //blank since we are used by shared_ptr
  Allocator& operator=( const Allocator& other ); //blank since we are used by shared_ptr

  //holds a reference to the real moab interface
  ::moab::ReadUtilIface* m_rface;

};

}
}
}

#endif
