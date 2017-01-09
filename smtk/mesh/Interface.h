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

#include "smtk/mesh/CellTraits.h"
#include "smtk/mesh/CellTypes.h"
#include "smtk/mesh/DimensionTypes.h"
#include "smtk/mesh/Handle.h"
#include "smtk/mesh/TypeSet.h"

#include <vector>

namespace smtk {
namespace mesh {

//forward declare classes we use
class ContainsFunctor;
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

  template<int CellType>
  bool allocateCells( std::size_t numCellsToAlloc,
                      smtk::mesh::HandleRange& createdCellIds,
                      smtk::mesh::Handle*& connectivityArray)
  {
  typedef typename smtk::mesh::CellEnumToType<CellType>::Traits Traits;
  smtk::mesh::CellType cellType = static_cast<smtk::mesh::CellType>(CellType);

  return this->allocateCells( cellType,
                              numCellsToAlloc,
                              Traits::NUM_VERTICES,
                              createdCellIds,
                              connectivityArray);
  }

  virtual bool connectivityModified( const smtk::mesh::HandleRange& cellsToUpdate,
                                     int numVertsPerCell,
                                     const smtk::mesh::Handle* connectivityArray) = 0;

};

//----------------------------------------------------------------------------
// BufferedCellAllocator allows for the allocation of meshes by
// 1. reserving memmory for all of your points
// 2. filling your points and cell connectivities by point index.
// Point indices are assumed to be contiguous with first index at 0.
class SMTKCORE_EXPORT BufferedCellAllocator
{
public:
  BufferedCellAllocator() : m_validState(false) {}

  virtual ~BufferedCellAllocator() {}

  virtual bool reserveNumberOfCoordinates(std::size_t nCoordinates) = 0;
  virtual bool setCoordinate(std::size_t coord, double* xyz) = 0;

  virtual bool addCell(smtk::mesh::CellType ctype, long long int* pointIds,
                       std::size_t nCoordinates = 0) = 0;
  virtual bool addCell(smtk::mesh::CellType ctype, long int* pointIds,
                       std::size_t nCoordinates = 0) = 0;
  virtual bool addCell(smtk::mesh::CellType ctype, int* pointIds,
                       std::size_t nCoordinates = 0) = 0;

  virtual bool flush() = 0;

  virtual smtk::mesh::HandleRange cells() = 0;

  bool setCoordinate(std::size_t coord, double x, double y, double z)
  { double xyz[3] = {x,y,z}; return this->setCoordinate(coord, xyz); }
  bool setCoordinate(std::size_t coord, float* xyz)
  { return this->setCoordinate(coord, xyz[0], xyz[1], xyz[2]); }
  bool setCoordinate(std::size_t coord, float x, float y, float z)
  { double xyz[3] = {x,y,z}; return this->setCoordinate(coord, xyz); }

  bool isValid() const { return this->m_validState; }

 protected:
  bool m_validState;
};

//----------------------------------------------------------------------------
// BufferedCellAllocator allows for the allocation of meshes by incrementally
// filling your points and cell connectivities by point index. This allocator is
// the least efficient for storage and data retrieval, but provides the most
// flexible API.
class SMTKCORE_EXPORT IncrementalAllocator
{
public:
  IncrementalAllocator() {}

  virtual ~IncrementalAllocator() {}

  virtual std::size_t addCoordinate(double* xyz) = 0;
  virtual bool setCoordinate(std::size_t coord, double* xyz) = 0;

  virtual bool addCell(smtk::mesh::CellType ctype, long long int* pointIds,
                       std::size_t nCoordinates = 0) = 0;
  virtual bool addCell(smtk::mesh::CellType ctype, long int* pointIds,
                       std::size_t nCoordinates = 0) = 0;
  virtual bool addCell(smtk::mesh::CellType ctype, int* pointIds,
                       std::size_t nCoordinates = 0) = 0;

  virtual bool flush() = 0;

  virtual smtk::mesh::HandleRange cells() = 0;

  std::size_t addCoordinate(double x, double y, double z)
  { double xyz[3] = {x,y,z}; return this->addCoordinate(xyz); }
  std::size_t addCoordinate(float* xyz)
  { return this->addCoordinate(xyz[0], xyz[1], xyz[2]); }
  std::size_t addCoordinate(float x, float y, float z)
  { double xyz[3] = {x,y,z}; return this->addCoordinate(xyz); }

  bool setCoordinate(std::size_t coord, double x, double y, double z)
  { double xyz[3] = {x,y,z}; return this->setCoordinate(coord, xyz); }
  bool setCoordinate(std::size_t coord, float* xyz)
  { return this->setCoordinate(coord, xyz[0], xyz[1], xyz[2]); }
  bool setCoordinate(std::size_t coord, float x, float y, float z)
  { double xyz[3] = {x,y,z}; return this->setCoordinate(coord, xyz); }

  virtual bool isValid() const = 0;
};

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT ConnectivityStorage
{
public:
  virtual ~ConnectivityStorage() { }

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
class SMTKCORE_EXPORT PointLocatorImpl
{
public:

  //copy of vector on return, ugggh.
  struct Results
    {
    Results():
      pointIds(),
      sqDistances(),
      x_s(), y_s(), z_s(),
      want_sqDistances(false),
      want_Coordinates(false)
    {
    }

    smtk::mesh::HandleRange pointIds;
    std::vector<double> sqDistances;
    std::vector<double> x_s, y_s, z_s;
    bool want_sqDistances;
    bool want_Coordinates;
    };

  virtual ~PointLocatorImpl() {}

  //returns all the point ids that are inside the locator
  virtual smtk::mesh::HandleRange range() const = 0;

  virtual void locatePointsWithinRadius(double x, double y, double z,
                                        double radius,
                                        Results& results) = 0;

};

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT Interface
{
public:
  Interface() {}

  virtual ~Interface() {}

  //---------------------------------------------------------------------------
  //get back a string that contains the pretty name for the interface class.
  //Requirements: The string must be all lower-case.
  //For example, the moab interface would return "moab"
  virtual std::string name() const = 0;

  //---------------------------------------------------------------------------
  //returns if the underlying data has been modified since the mesh was loaded
  //from disk. If the mesh has no underlying file, it will always be considered
  //modified. Once the mesh is written to disk, we will reset the modified
  //flag.
  virtual bool isModified() const = 0;

  //----------------------------------------------------------------------------
  //get back a lightweight interface around allocating memory into the given
  //interface. This is generally used to create new coordinates or cells that
  //are than assigned to an existing mesh or new mesh.
  //
  //If the current interface is read-only, the AllocatorPtr that is returned
  //will be NULL.
  //
  //Note: Merely fetching a valid allocator will mark the collection as
  //modified. This is done instead of on a per-allocation basis so that
  //modification state changes don't impact performance.
  virtual smtk::mesh::AllocatorPtr allocator() = 0;

  //----------------------------------------------------------------------------
  //get back a lightweight interface around incrementally allocating memory into
  //the given interface. This is generally used to create new coordinates or
  //cells that are than assigned to an existing mesh or new mesh.
  //
  //If the current interface is read-only, the BufferedCellAllocatorPtr that is
  //returned will be NULL.
  //
  //Note: Merely fetching a valid allocator will mark the collection as
  //modified. This is done instead of on a per-allocation basis so that
  //modification state changes don't impact performance.
  virtual smtk::mesh::BufferedCellAllocatorPtr bufferedCellAllocator() = 0;

  //----------------------------------------------------------------------------
  //get back a lightweight interface around incrementally allocating memory into
  //the given interface. This is generally used to create new coordinates or
  //cells that are than assigned to an existing mesh or new mesh.
  //
  //If the current interface is read-only, the IncrementalAllocatorPtr that is
  //returned will be NULL.
  //
  //Note: Merely fetching a valid allocator will mark the collection as
  //modified. This is done instead of on a per-allocation basis so that
  //modification state changes don't impact performance.
  virtual smtk::mesh::IncrementalAllocatorPtr incrementalAllocator() = 0;

  //----------------------------------------------------------------------------
  //get back an efficient storage mechanism for a range of cells point
  //connectivity. This allows for efficient iteration of cell connectivity, and
  //conversion to other formats
  virtual smtk::mesh::ConnectivityStoragePtr connectivityStorage(const smtk::mesh::HandleRange& cells) = 0;

  //----------------------------------------------------------------------------
  //get back an efficient point locator for a range of points
  //This allows for efficient point locator on a per interface basis.
  virtual smtk::mesh::PointLocatorImplPtr pointLocator(const smtk::mesh::HandleRange& points) = 0;
  virtual smtk::mesh::PointLocatorImplPtr pointLocator(const double* const xyzs, std::size_t numPoints, bool ignoreZValues=false) = 0;
  virtual smtk::mesh::PointLocatorImplPtr pointLocator(const float* const xyzs, std::size_t numPoints, bool ignoreZValues=false) = 0;

  //----------------------------------------------------------------------------
  virtual smtk::mesh::Handle getRoot() const = 0;

  //----------------------------------------------------------------------------
  //creates a mesh with that contains the input cells.
  //the mesh will have the root as its parent.
  //Will fail if the HandleRange is empty or doesn't contain valid
  //cell handles.
  //Note: Will mark the interface as modified when successful
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
  //get all the coordinates for the points in this range
  //xyz needs to be allocated to 3*points.size()
  virtual bool getCoordinates(const smtk::mesh::HandleRange& points,
                              double* xyz) const = 0;

  //----------------------------------------------------------------------------
  //get all the coordinates for the points in this range
  //xyz needs to be allocated to 3*points.size()
  //Floats are not how we store the coordinates internally, so asking for
  //the coordinates in such a manner could cause data inaccuracies to appear
  //so generally this is only used if you fully understand the input domain
  virtual bool getCoordinates(const smtk::mesh::HandleRange& points,
                              float* xyz) const = 0;

  //----------------------------------------------------------------------------
  //set all the coordinates for the points in this range
  //xyz needs to be allocated to 3*points.size()
  virtual bool setCoordinates(const smtk::mesh::HandleRange& points,
                              const double* const xyz) = 0;

  //----------------------------------------------------------------------------
  //set all the coordinates for the points in this range
  //xyz needs to be allocated to 3*points.size()
  virtual bool setCoordinates(const smtk::mesh::HandleRange& points,
                              const float* const xyz) = 0;

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
  //merge any duplicate points used by the cells that have been passed
  //Note: Will mark the interface as modified when successful
  virtual bool mergeCoincidentContactPoints(const smtk::mesh::HandleRange& meshes,
                                           double tolerance) = 0;

  //----------------------------------------------------------------------------
  // Note: Will mark the interface as modified when successful
  virtual bool setDomain(const smtk::mesh::HandleRange& meshsets,
                         const smtk::mesh::Domain& domain) const = 0;

  //----------------------------------------------------------------------------
  // Note: Will mark the interface as modified when successful
  virtual bool setDirichlet(const smtk::mesh::HandleRange& meshsets,
                            const smtk::mesh::Dirichlet& dirichlet) const = 0;

  //----------------------------------------------------------------------------
  // Note: Will mark the interface as modified when successful
  virtual bool setNeumann(const smtk::mesh::HandleRange& meshsets,
                          const smtk::mesh::Neumann& neumann) const = 0;

  //----------------------------------------------------------------------------
  // Specify for a given sets of handles what the associated model entity is.
  // This allows for a model region, face, or edge to be associated with a
  // given set of meshes.
  // Note: Only MeshSets can have associations applied. Other elements will cause
  // undefined behavior
  // Note: Will mark the interface as modified when successful
  virtual bool setAssociation(const smtk::common::UUID& modelUUID,
                              const smtk::mesh::HandleRange& meshsets) const = 0;

  //----------------------------------------------------------------------------
  // For a given handle root, find all meshsets that have an association to
  // the given model uuid
  virtual smtk::mesh::HandleRange findAssociations(const smtk::mesh::Handle& root,
                                                   const smtk::common::UUID& modelUUID) const = 0;

  //----------------------------------------------------------------------------
  // Specify the model uuid that the root of this interface is associated too
  // This represents generally is the MODEL_ENTITY that owns all associations
  // with this interface
  // Note: Will mark the interface as modified when successful
  virtual bool setRootAssociation(const smtk::common::UUID& modelUUID) const = 0;

  //----------------------------------------------------------------------------
  virtual smtk::common::UUID rootAssociation() const = 0;

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
  virtual void cellForEach( const HandleRange& cells,
                            smtk::mesh::PointConnectivity& a,
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
  //Note: Will mark the interface as modified when successful
  virtual bool deleteHandles(const smtk::mesh::HandleRange& toDel) = 0;

  //----------------------------------------------------------------------------
  //Manually modify the modified state. This is only done to set the modified
  //state to be proper after serialization / deserialization.
  virtual void setModifiedState(bool state)=0;

};

}
}

#endif
