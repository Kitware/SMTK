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
#ifndef __smtk_mesh_Interface_h
#define __smtk_mesh_Interface_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/CellTypes.h"
#include "smtk/mesh/DimensionTypes.h"
#include "smtk/mesh/Handle.h"
#include "smtk/mesh/TypeSet.h"

#include <vector>

namespace smtk {
namespace mesh {

//forward declare classes we use
struct ContainsFunctor;

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT Allocator
{
public:
  virtual  ~Allocator() {};

  virtual bool allocatePoints( std::size_t numPointsToAlloc,
                               smtk::mesh::Handle& firstVertexHandle,
                               std::vector<double* >& coordinateMemory) = 0;

  virtual bool allocateCells( smtk::mesh::CellType cellType,
                              std::size_t numCellsToAlloc,
                              int numVertsPerCell,
                              smtk::mesh::HandleRange& createdCellIds,
                              smtk::mesh::Handle* connectivityArray) = 0;

  virtual bool connectivityModified( const smtk::mesh::HandleRange& cellsToUpdate,
                                     int numVertsPerCell,
                                     smtk::mesh::Handle* connectivityArray) = 0;

};

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT Interface
{
public:
  Interface() {};

  virtual ~Interface() {};

  //----------------------------------------------------------------------------
  //get back a lightweight interface around allocating memory into the given
  //interface. This is generally used to create new coordinates or cells that
  //are than assigned to an existing mesh or new mesh
  virtual smtk::mesh::AllocatorPtr allocator() = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::Handle get_root() = 0;

  //----------------------------------------------------------------------------
  //creates a mesh with that contains the input cells.
  //the mesh will have the root as its parent.
  //this function needs to be expanded to support parenting to other handles
  //this function needs to be expanded to support adding tags to the mesh
  virtual bool create_mesh(smtk::mesh::HandleRange cells,
                           smtk::mesh::Handle& meshHandle) = 0;

  //----------------------------------------------------------------------------
  //the number of meshes that are children of this mesh.
  virtual std::size_t numMeshes(smtk::mesh::Handle handle) = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::HandleRange get_meshsets(smtk::mesh::Handle handle) = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::HandleRange get_meshsets(smtk::mesh::Handle handle,
                                               int dimension) = 0;

  //----------------------------------------------------------------------------
  //find all entity sets that have this exact name tag
  virtual smtk::mesh::HandleRange get_meshsets(smtk::mesh::Handle handle,
                                               const std::string& name) = 0;

  //----------------------------------------------------------------------------
  //get all cells held by this range
  virtual smtk::mesh::HandleRange get_cells(smtk::mesh::HandleRange meshsets) = 0;

  //----------------------------------------------------------------------------
  //get all cells held by this range handle of a given cell type
  virtual smtk::mesh::HandleRange get_cells(smtk::mesh::HandleRange meshsets,
                                            smtk::mesh::CellType cellType) = 0;

  //----------------------------------------------------------------------------
  //get all cells held by this range handle of a given cell type(s)
  virtual smtk::mesh::HandleRange get_cells(smtk::mesh::HandleRange meshsets,
                                            const smtk::mesh::CellTypes& cellTypes) = 0;

  //----------------------------------------------------------------------------
  //get all cells held by this range handle of a given dimension
  virtual smtk::mesh::HandleRange get_cells(smtk::mesh::HandleRange meshsets,
                                            smtk::mesh::DimensionType dim) = 0;

  //----------------------------------------------------------------------------
  virtual std::vector< std::string > compute_names(const smtk::mesh::HandleRange& r) = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::TypeSet compute_types(smtk::mesh::Handle handle) = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::HandleRange set_intersect(const smtk::mesh::HandleRange& a,
                                                const smtk::mesh::HandleRange& b) const = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::HandleRange set_difference(const smtk::mesh::HandleRange& a,
                                                 const smtk::mesh::HandleRange& b) const = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::HandleRange set_union(const smtk::mesh::HandleRange& a,
                                            const smtk::mesh::HandleRange& b) const = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::HandleRange point_intersect(const smtk::mesh::HandleRange& a,
                                                  const smtk::mesh::HandleRange& b,
                                                  const smtk::mesh::ContainsFunctor& containsFunctor) = 0;
  //----------------------------------------------------------------------------
  virtual smtk::mesh::HandleRange point_difference(const smtk::mesh::HandleRange& a,
                                                   const smtk::mesh::HandleRange& b,
                                                   const smtk::mesh::ContainsFunctor& containsFunctor) = 0;
};

}
}

#endif
