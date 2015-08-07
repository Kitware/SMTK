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

#include "smtk/CoreExports.h"
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
class SMTKCORE_EXPORT ConnectivityStorage
{
public:
  //struct that holds the required information to compute what is the
  //current cell when we are iterating.
  struct IterationState
    {
    IterationState():
      whichConnectivityVector(0),
      ptrOffsetInVector(0)
      {
      }
    std::size_t whichConnectivityVector;
    std::size_t ptrOffsetInVector;
    };

  virtual void initTraversal( IterationState& state ) = 0;

  virtual bool fetchNextCell( IterationState& state,
                      smtk::mesh::CellType& cellType,
                      int& numPts,
                      const smtk::mesh::Handle* &points) = 0;

  virtual bool equal( ConnectivityStorage* other ) const = 0;

  virtual std::size_t cellSize() const = 0;

  virtual std::size_t vertSize() const = 0;
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
  //get back an efficient storage mechanism for a range of cells point
  //connectivity. This allows for efficient iteration of cell connectivity, and
  //conversion to other formats
  virtual smtk::mesh::ConnectivityStoragePtr connectivityStorage(const smtk::mesh::HandleRange& cells) = 0;

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
  //find all entity sets that have this exact domain tag
  virtual smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle,
                                              const smtk::mesh::Domain& domain) const = 0;

  //----------------------------------------------------------------------------
  //find all entity sets that have this exact dirichlet tag
  virtual smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle,
                                              const smtk::mesh::Dirichlet& dirichlet) const = 0;

  //----------------------------------------------------------------------------
  //find all entity sets that have this exact neumann tag
  virtual smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle,
                                              const smtk::mesh::Neumann& neumann) const = 0;

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
  //get all points held by this range of handle of a given dimension
  virtual smtk::mesh::HandleRange getPoints(const smtk::mesh::HandleRange& cells) const = 0;

  //----------------------------------------------------------------------------
  virtual std::vector< std::string > computeNames(const smtk::mesh::HandleRange& meshsets) const = 0;

  //----------------------------------------------------------------------------
  virtual std::vector< smtk::mesh::Domain > computeDomainValues( const smtk::mesh::HandleRange& meshsets) const = 0;

  //----------------------------------------------------------------------------
  virtual std::vector< smtk::mesh::Dirichlet > computeDirichletValues(const smtk::mesh::HandleRange& meshsets) const = 0;

  //----------------------------------------------------------------------------
  virtual std::vector< smtk::mesh::Neumann > computeNeumannValues(const smtk::mesh::HandleRange& meshsets) const = 0;

  //----------------------------------------------------------------------------
  virtual smtk::common::UUIDArray computeModelEntities(const smtk::mesh::HandleRange& meshsets) const = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::TypeSet computeTypes(const smtk::mesh::HandleRange& range) const = 0;

  //----------------------------------------------------------------------------
  //compute the cells that make the shell/skin of the set of meshes
  virtual bool computeShell(const smtk::mesh::HandleRange& meshes, smtk::mesh::HandleRange& shell) const = 0;

  //----------------------------------------------------------------------------
  virtual bool setDomain(const smtk::mesh::HandleRange& meshsets,
                         const smtk::mesh::Domain& domain) const = 0;

  //----------------------------------------------------------------------------
  virtual bool setDirichlet(const smtk::mesh::HandleRange& meshsets,
                            const smtk::mesh::Dirichlet& dirichlet) const = 0;

  //----------------------------------------------------------------------------
  virtual bool setNeumann(const smtk::mesh::HandleRange& meshsets,
                          const smtk::mesh::Neumann& neumann) const = 0;

  //----------------------------------------------------------------------------
  virtual bool setModelEntity(
    const smtk::mesh::HandleRange& meshsets,
    const smtk::common::UUID& uuid) const = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::HandleRange findAssociations(
    const smtk::mesh::Handle& root,
    const smtk::common::UUID& modelUUID) = 0;

  //----------------------------------------------------------------------------
  virtual bool addAssociation(const smtk::common::UUID& modelUUID,
                              const smtk::mesh::HandleRange& range) = 0;

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
  virtual void pointForEach( const HandleRange &points,
                             smtk::mesh::PointForEach& filter) const = 0;

  //----------------------------------------------------------------------------
  virtual void cellForEach( smtk::mesh::PointConnectivity& a,
                            smtk::mesh::CellForEach& filter) const = 0;

  //----------------------------------------------------------------------------
  virtual void meshForEach( const HandleRange &meshes,
                            smtk::mesh::MeshForEach& filter) const = 0;

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
