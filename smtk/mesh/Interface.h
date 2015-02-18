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

class PointConnectivity;

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT Allocator
{
public:
  virtual ~Allocator() {}

  virtual bool allocatePoints( std::size_t numPointsToAlloc,
                               smtk::mesh::Handle& firstVertexHandle,
                               std::vector<double* >& coordinateMemory) = 0;

  virtual bool allocateCells( smtk::mesh::CellType cellType,
                              std::size_t numCellsToAlloc,
                              int numVertsPerCell,
                              smtk::mesh::HandleRange& createdCellIds,
                              smtk::mesh::Handle*& connectivityArray) = 0;

  virtual bool connectivityModified( const smtk::mesh::HandleRange& cellsToUpdate,
                                     int numVertsPerCell,
                                     const smtk::mesh::Handle* connectivityArray) = 0;

};

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT Interface
{
public:
  Interface() {}

  virtual ~Interface() {}

  //----------------------------------------------------------------------------
  //get back a lightweight interface around allocating memory into the given
  //interface. This is generally used to create new coordinates or cells that
  //are than assigned to an existing mesh or new mesh
  virtual smtk::mesh::AllocatorPtr allocator() = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::Handle getRoot() const = 0;

  //----------------------------------------------------------------------------
  //creates a mesh with that contains the input cells.
  //the mesh will have the root as its parent.
  //Will fail if the HandleRange is empty or doesn't contain valid
  //cell handles.
  virtual bool createMesh(const smtk::mesh::HandleRange& cells,
                          smtk::mesh::Handle& meshHandle) = 0;

  //----------------------------------------------------------------------------
  //the number of meshes that are children of this mesh.
  virtual std::size_t numMeshes(smtk::mesh::Handle handle) const = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle) const = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle,
                                               int dimension) const = 0;

  //----------------------------------------------------------------------------
  //find all entity sets that have this exact name tag
  virtual smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle,
                                               const std::string& name) const = 0;

  //----------------------------------------------------------------------------
  //find all entity sets that have this exact material tag
  virtual smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle,
                                              const smtk::mesh::Material& material) const = 0;

  //----------------------------------------------------------------------------
  //find all entity sets that have this exact dirichlet tag
  virtual smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle,
                                              const smtk::mesh::Dirichlet& dirichlet) const = 0;

  //----------------------------------------------------------------------------
  //get all cells held by this range
  virtual smtk::mesh::HandleRange getCells(const smtk::mesh::HandleRange& meshsets) const = 0;

  //----------------------------------------------------------------------------
  //get all cells held by this range handle of a given cell type
  virtual smtk::mesh::HandleRange getCells(const smtk::mesh::HandleRange& meshsets,
                                           smtk::mesh::CellType cellType) const = 0;

  //----------------------------------------------------------------------------
  //get all cells held by this range handle of a given cell type(s)
  virtual smtk::mesh::HandleRange getCells(const smtk::mesh::HandleRange& meshsets,
                                           const smtk::mesh::CellTypes& cellTypes) const = 0;

  //----------------------------------------------------------------------------
  //get all cells held by this range handle of a given dimension
  virtual smtk::mesh::HandleRange getCells(const smtk::mesh::HandleRange& meshsets,
                                           smtk::mesh::DimensionType dim) const = 0;

  //----------------------------------------------------------------------------
  virtual std::vector< std::string > computeNames(const smtk::mesh::HandleRange& meshsets) const = 0;

  //----------------------------------------------------------------------------
  virtual std::vector< smtk::mesh::Material > computeMaterialValues( const smtk::mesh::HandleRange& meshsets) const = 0;

  //----------------------------------------------------------------------------
  virtual std::vector< smtk::mesh::Dirichlet > computeDirichletValues(const smtk::mesh::HandleRange& meshsets) const = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::TypeSet computeTypes(smtk::mesh::Handle handle) const = 0;

  //----------------------------------------------------------------------------
  //compute the cells that make the shell/skin of the set of meshes
  virtual bool computeShell(const smtk::mesh::HandleRange& meshes, smtk::mesh::HandleRange& shell) const = 0;

  //----------------------------------------------------------------------------
  virtual bool setMaterial(const smtk::mesh::HandleRange& meshsets,
                           const smtk::mesh::Material& material) const = 0;

  //----------------------------------------------------------------------------
  virtual bool setDirichlet(const smtk::mesh::HandleRange& meshsets,
                            const smtk::mesh::Dirichlet& dirichlet) const = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::HandleRange rangeIntersect(const smtk::mesh::HandleRange& a,
                                                 const smtk::mesh::HandleRange& b) const = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::HandleRange rangeDifference(const smtk::mesh::HandleRange& a,
                                                  const smtk::mesh::HandleRange& b) const = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::HandleRange rangeUnion(const smtk::mesh::HandleRange& a,
                                             const smtk::mesh::HandleRange& b) const = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::HandleRange pointIntersect(const smtk::mesh::HandleRange& a,
                                                 const smtk::mesh::HandleRange& b,
                                                 smtk::mesh::PointConnectivity& bpc,
                                                 const smtk::mesh::ContainsFunctor& containsFunctor) const = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::HandleRange pointDifference(const smtk::mesh::HandleRange& a,
                                                  const smtk::mesh::HandleRange& b,
                                                  smtk::mesh::PointConnectivity& bpc,
                                                  const smtk::mesh::ContainsFunctor& containsFunctor) const = 0;

  //----------------------------------------------------------------------------
  //The handles must be all mesh or cell elements. Mixed ranges wil
  //not be deleted and will return false. Empty ranges will be ignored
  //and return true.
  //When deleting meshes if the range contains the root handle (getRoot()) the
  //request will fail, nothing will be deleted, and we will return false.
  virtual bool deleteHandles(const smtk::mesh::HandleRange& toDel) = 0;
};

}
}

#endif
