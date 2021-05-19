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
#ifndef __smtk_mesh_json_Interface_h
#define __smtk_mesh_json_Interface_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/core/CellTypes.h"
#include "smtk/mesh/core/DimensionTypes.h"
#include "smtk/mesh/core/Handle.h"
#include "smtk/mesh/core/Interface.h"
#include "smtk/mesh/core/TypeSet.h"

#include "smtk/mesh/json/MeshInfo.h"

#include <vector>

namespace smtk
{
namespace mesh
{
namespace json
{
//construct an empty interface instance
SMTKCORE_EXPORT
smtk::mesh::json::InterfacePtr make_interface();

/// Concrete implementation for json serialization
class SMTKCORE_EXPORT Interface : public smtk::mesh::Interface
{
public:
  Interface();

  Interface(const std::vector<smtk::mesh::json::MeshInfo>& info);

  ~Interface() override;

  //returns if the underlying data has been modified since the mesh was loaded
  //from disk. If the mesh has no underlying file, it will always be considered
  //modified. Once the mesh is written to disk, we will reset the modified
  //flag.
  bool isModified() const override;

  //get back a string that contains the pretty name for the interface class.
  //Requirements: The string must be all lower-case.
  std::string name() const override { return std::string("json"); }

  //Add more meshes to the Interface
  void addMeshes(const std::vector<smtk::mesh::json::MeshInfo>& info);

  //get back a lightweight interface around allocating memory into the given
  //interface. This is generally used to create new coordinates or cells that
  //are than assigned to an existing mesh or new mesh
  //
  //If the current interface is read-only, the AllocatorPtr that is returned
  //will be nullptr.
  //
  //Note: Merely fetching a valid allocator will mark the resource as
  //modified. This is done instead of on a per-allocation basis so that
  //modification state changes don't impact performance.
  smtk::mesh::AllocatorPtr allocator() override;

  //get back a lightweight interface around incrementally allocating memory into
  //the given interface. This is generally used to create new coordinates or
  //cells that are than assigned to an existing mesh or new mesh.
  //
  //If the current interface is read-only, the BufferedCellAllocatorPtr that is
  //returned will be nullptr.
  //
  //Note: Merely fetching a valid allocator will mark the resource as
  //modified. This is done instead of on a per-allocation basis so that
  //modification state changes don't impact performance.
  smtk::mesh::BufferedCellAllocatorPtr bufferedCellAllocator() override;

  //get back a lightweight interface around incrementally allocating memory into
  //the given interface. This is generally used to create new coordinates or
  //cells that are than assigned to an existing mesh or new mesh.
  //
  //If the current interface is read-only, the IncrementalAllocatorPtr that is
  //returned will be nullptr.
  //
  //Note: Merely fetching a valid allocator will mark the resource as
  //modified. This is done instead of on a per-allocation basis so that
  //modification state changes don't impact performance.
  smtk::mesh::IncrementalAllocatorPtr incrementalAllocator() override;

  //get back an efficient storage mechanism for a range of cells point
  //connectivity. This allows for efficient iteration of cell connectivity, and
  //conversion to other formats
  smtk::mesh::ConnectivityStoragePtr connectivityStorage(
    const smtk::mesh::HandleRange& cells) override;

  //get back an efficient point locator for a range of points
  //This allows for efficient point locator on a per interface basis.
  smtk::mesh::PointLocatorImplPtr pointLocator(const smtk::mesh::HandleRange& points) override;
  smtk::mesh::PointLocatorImplPtr pointLocator(
    std::size_t numPoints,
    const std::function<std::array<double, 3>(std::size_t)>& coordinates) override;

  smtk::mesh::Handle getRoot() const override;

  void registerQueries(smtk::mesh::Resource&) const override;

  //creates a mesh with that contains the input cells.
  //the mesh will have the root as its parent.
  //The mesh will be tagged with the GEOM_DIMENSION tag with a value that is
  //equal to highest dimension of cell inside
  //Will fail if the HandleRange is empty or doesn't contain valid
  //cell handles.
  bool createMesh(const smtk::mesh::HandleRange& cells, smtk::mesh::Handle& meshHandle) override;

  std::size_t numMeshes(smtk::mesh::Handle handle) const override;

  smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle) const override;

  smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle, int dimension) const override;

  //find all entity sets that have this exact name tag
  smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle, const std::string& name)
    const override;

  //find all entity sets that have this exact domain tag
  smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle, const smtk::mesh::Domain& domain)
    const override;

  //find all entity sets that have this exact dirichlet tag
  smtk::mesh::HandleRange getMeshsets(
    smtk::mesh::Handle handle,
    const smtk::mesh::Dirichlet& dirichlet) const override;

  //find all entity sets that have this exact neumann tag
  smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle, const smtk::mesh::Neumann& neumann)
    const override;

  //get all cells held by this range
  smtk::mesh::HandleRange getCells(const smtk::mesh::HandleRange& meshsets) const override;

  //get all cells held by this range handle of a given cell type
  smtk::mesh::HandleRange getCells(
    const smtk::mesh::HandleRange& meshsets,
    smtk::mesh::CellType cellType) const override;

  //get all cells held by this range handle of a given cell type(s)
  smtk::mesh::HandleRange getCells(
    const smtk::mesh::HandleRange& meshsets,
    const smtk::mesh::CellTypes& cellTypes) const override;

  //get all cells held by this range handle of a given dimension
  smtk::mesh::HandleRange getCells(
    const smtk::mesh::HandleRange& meshsets,
    smtk::mesh::DimensionType dim) const override;

  //get all points held by this range of handle of a given dimension. If
  //boundary_only is set to true, ignore the higher order points of the
  //cells
  smtk::mesh::HandleRange getPoints(
    const smtk::mesh::HandleRange& cells,
    bool boundary_only = false) const override;

  //get all the coordinates for the points in this range
  //xyz needs to be allocated to 3*points.size()
  //Floats are not how we store the coordinates internally, so asking for
  //the coordinates in such a manner could cause data inaccuracies to appear
  //so generally this is only used if you fully understand the input domain
  bool getCoordinates(const smtk::mesh::HandleRange& points, double* xyz) const override;

  //get all the coordinates for the points in this range
  //xyz needs to be allocated to 3*points.size()
  bool getCoordinates(const smtk::mesh::HandleRange& points, float* xyz) const override;

  //set all the coordinates for the points in this range
  //xyz needs to be allocated to 3*points.size()
  bool setCoordinates(const smtk::mesh::HandleRange& points, const double* xyz) override;

  //set all the coordinates for the points in this range
  //xyz needs to be allocated to 3*points.size()
  bool setCoordinates(const smtk::mesh::HandleRange& points, const float* xyz) override;

  std::vector<std::string> computeNames(const smtk::mesh::HandleRange& meshsets) const override;

  std::string name(const smtk::mesh::Handle& meshset) const override;
  bool setName(const smtk::mesh::Handle& meshset, const std::string& name) override;

  std::vector<smtk::mesh::Domain> computeDomainValues(
    const smtk::mesh::HandleRange& meshsets) const override;

  std::vector<smtk::mesh::Dirichlet> computeDirichletValues(
    const smtk::mesh::HandleRange& meshsets) const override;

  std::vector<smtk::mesh::Neumann> computeNeumannValues(
    const smtk::mesh::HandleRange& meshsets) const override;

  smtk::common::UUIDArray computeModelEntities(
    const smtk::mesh::HandleRange& meshsets) const override;

  smtk::mesh::TypeSet computeTypes(const smtk::mesh::HandleRange& range) const override;

  //compute the cells that make the shell/skin of the set of meshes
  bool computeShell(const smtk::mesh::HandleRange& meshes, smtk::mesh::HandleRange& shell)
    const override;

  //compute adjacencies of a given dimension, creating them if necessary
  bool computeAdjacenciesOfDimension(
    const smtk::mesh::HandleRange& meshes,
    int dimension,
    smtk::mesh::HandleRange& adj) const override;

  //given a handle to a cell, return its parent handle and canonical index.
  bool canonicalIndex(const smtk::mesh::Handle& cell, smtk::mesh::Handle& parent, int& index)
    const override;

  //given a handle to a cell, return its dimension-equivalent neighbors.
  smtk::mesh::HandleRange neighbors(const smtk::mesh::Handle& cell) const override;

  //merge any duplicate points used by the cells that have been passed
  bool mergeCoincidentContactPoints(const smtk::mesh::HandleRange& meshes, double tolerance)
    override;

  bool setDomain(const smtk::mesh::HandleRange& meshsets, const smtk::mesh::Domain& domain)
    const override;

  bool setDirichlet(const smtk::mesh::HandleRange& meshsets, const smtk::mesh::Dirichlet& dirichlet)
    const override;

  bool setNeumann(const smtk::mesh::HandleRange& meshsets, const smtk::mesh::Neumann& neumann)
    const override;

  bool setId(const smtk::mesh::Handle& meshset, const smtk::common::UUID& id) const override;

  smtk::common::UUID getId(const smtk::mesh::Handle& meshset) const override;

  bool findById(
    const smtk::mesh::Handle& root,
    const smtk::common::UUID& id,
    smtk::mesh::Handle& meshset) const override;

  bool setAssociation(const smtk::common::UUID& modelUUID, const smtk::mesh::HandleRange& meshsets)
    const override;

  smtk::mesh::HandleRange findAssociations(
    const smtk::mesh::Handle& root,
    const smtk::common::UUID& modelUUID) const override;

  bool setRootAssociation(const smtk::common::UUID& modelUUID) const override;

  smtk::common::UUID rootAssociation() const override;

  bool createCellField(
    const smtk::mesh::HandleRange& meshsets,
    const std::string& name,
    std::size_t dimension,
    const smtk::mesh::FieldType& type,
    const void* data) override;

  int getCellFieldDimension(const smtk::mesh::CellFieldTag& cfTag) const override;
  smtk::mesh::FieldType getCellFieldType(const smtk::mesh::CellFieldTag& pfTag) const override;

  smtk::mesh::HandleRange getMeshsets(
    smtk::mesh::Handle handle,
    const smtk::mesh::CellFieldTag& cfTag) const override;

  bool hasCellField(const smtk::mesh::HandleRange& meshsets, const smtk::mesh::CellFieldTag& cfTag)
    const override;

  bool getCellField(
    const smtk::mesh::HandleRange& meshsets,
    const smtk::mesh::CellFieldTag& cfTag,
    void* data) const override;

  bool setCellField(
    const smtk::mesh::HandleRange& meshsets,
    const smtk::mesh::CellFieldTag& cfTag,
    const void* const data) override;

  bool getField(
    const smtk::mesh::HandleRange& cells,
    const smtk::mesh::CellFieldTag& cfTag,
    void* data) const override;

  bool setField(
    const smtk::mesh::HandleRange& cells,
    const smtk::mesh::CellFieldTag& cfTag,
    const void* const data) override;

  std::set<smtk::mesh::CellFieldTag> computeCellFieldTags(
    const smtk::mesh::Handle& handle) const override;

  bool deleteCellField(
    const smtk::mesh::CellFieldTag& cfTag,
    const smtk::mesh::HandleRange& meshsets) override;

  bool createPointField(
    const smtk::mesh::HandleRange& meshsets,
    const std::string& name,
    std::size_t dimension,
    const smtk::mesh::FieldType& type,
    const void* data) override;

  int getPointFieldDimension(const smtk::mesh::PointFieldTag& pfTag) const override;
  smtk::mesh::FieldType getPointFieldType(const smtk::mesh::PointFieldTag& pfTag) const override;

  smtk::mesh::HandleRange getMeshsets(
    smtk::mesh::Handle handle,
    const smtk::mesh::PointFieldTag& pfTag) const override;

  bool hasPointField(
    const smtk::mesh::HandleRange& meshsets,
    const smtk::mesh::PointFieldTag& pfTag) const override;

  bool getPointField(
    const smtk::mesh::HandleRange& meshsets,
    const smtk::mesh::PointFieldTag& pfTag,
    void* data) const override;

  bool setPointField(
    const smtk::mesh::HandleRange& meshsets,
    const smtk::mesh::PointFieldTag& pfTag,
    const void* const data) override;

  bool getField(
    const smtk::mesh::HandleRange& points,
    const smtk::mesh::PointFieldTag& pfTag,
    void* data) const override;

  bool setField(
    const smtk::mesh::HandleRange& points,
    const smtk::mesh::PointFieldTag& pfTag,
    const void* const data) override;

  std::set<smtk::mesh::PointFieldTag> computePointFieldTags(
    const smtk::mesh::Handle& handle) const override;

  bool deletePointField(
    const smtk::mesh::PointFieldTag& pfTag,
    const smtk::mesh::HandleRange& meshsets) override;

  smtk::mesh::HandleRange pointIntersect(
    const smtk::mesh::HandleRange& a,
    const smtk::mesh::HandleRange& b,
    smtk::mesh::PointConnectivity& bpc,
    smtk::mesh::ContainmentType t) const override;

  smtk::mesh::HandleRange pointDifference(
    const smtk::mesh::HandleRange& a,
    const smtk::mesh::HandleRange& b,
    smtk::mesh::PointConnectivity& bpc,
    smtk::mesh::ContainmentType t) const override;

  void pointForEach(const HandleRange& points, smtk::mesh::PointForEach& filter) const override;

  void cellForEach(
    const HandleRange& cells,
    smtk::mesh::PointConnectivity& pc,
    smtk::mesh::CellForEach& filter) const override;

  void meshForEach(const HandleRange& meshes, smtk::mesh::MeshForEach& filter) const override;

  bool deleteHandles(const smtk::mesh::HandleRange& toDel) override;

  void setModifiedState(bool state) override { m_modified = state; }

private:
  typedef std::vector<smtk::mesh::json::MeshInfo> MeshInfoVecType;

  MeshInfoVecType::const_iterator find(smtk::mesh::Handle handle) const;

  std::vector<smtk::mesh::json::MeshInfo> m_meshInfo;

  //this is ugly, but as this is the only state we have I am going to roll
  //with it. If we start adding more member variables, we should offload it
  //all to an internal class
  mutable smtk::common::UUID m_associated_model;
  mutable bool m_modified;
};
} // namespace json
} // namespace mesh
} // namespace smtk

#endif
