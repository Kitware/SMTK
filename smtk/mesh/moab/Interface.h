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
#ifndef __smtk_mesh_moab_Interface_h
#define __smtk_mesh_moab_Interface_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/Interface.h"
#include "smtk/mesh/CellTypes.h"
#include "smtk/mesh/DimensionTypes.h"
#include "smtk/mesh/Handle.h"
#include "smtk/mesh/TypeSet.h"

#include <vector>

namespace moab
{
  class Interface;
}

namespace smtk {
namespace mesh {
namespace moab
{
//construct an empty interface instance, this is properly connected
//to a moab database
//----------------------------------------------------------------------------
SMTKCORE_EXPORT
smtk::mesh::moab::InterfacePtr make_interface();

//Given a smtk::mesh Collection extract the underlying smtk::mesh::moab interface
//from it. This requires that the collection was created with the proper interface
//to begin with.
//----------------------------------------------------------------------------
smtk::mesh::moab::InterfacePtr extract_interface( const smtk::mesh::CollectionPtr& c);

//Given a smtk::mesh Interface convert it to a smtk::mesh::moab interface, and than
//extract the raw moab interface pointer from that
//----------------------------------------------------------------------------
SMTKCORE_EXPORT
::moab::Interface* extract_moab_interface( const smtk::mesh::InterfacePtr &iface);

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT Interface : public smtk::mesh::Interface
{
public:
  Interface();

  virtual ~Interface();

  //---------------------------------------------------------------------------
  //get back a string that contains the pretty name for the interface class.
  //Requirements: The string must be all lower-case.
  virtual std::string name() const { return std::string("moab"); }

  //---------------------------------------------------------------------------
  //returns if the underlying data has been modified since the mesh was loaded
  //from disk. If the mesh has no underlying file, it will always be considered
  //modified. Once the mesh is written to disk, we will reset the modified
  //flag.
  virtual bool isModified() const;

  //----------------------------------------------------------------------------
  //get back a lightweight interface around allocating memory into the given
  //interface. This is generally used to create new coordinates or cells that
  //are than assigned to an existing mesh or new mesh
  //
  //If the current interface is read-only, the AllocatorPtr that is returned
  //will be NULL.
  //
  //Note: Merely fetching a valid allocator will mark the collection as
  //modified. This is done instead of on a per-allocation basis so that
  //modification state changes don't impact performance.
  smtk::mesh::AllocatorPtr allocator();

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
  smtk::mesh::BufferedCellAllocatorPtr bufferedCellAllocator();

  //----------------------------------------------------------------------------
  //get back an efficient storage mechanism for a range of cells point
  //connectivity. This allows for efficient iteration of cell connectivity, and
  //conversion to other formats
  smtk::mesh::ConnectivityStoragePtr connectivityStorage(const smtk::mesh::HandleRange& cells);

  //----------------------------------------------------------------------------
  //get back an efficient point locator for a range of points
  //This allows for efficient point locator on a per interface basis.
  smtk::mesh::PointLocatorImplPtr pointLocator(const smtk::mesh::HandleRange& points);
  smtk::mesh::PointLocatorImplPtr pointLocator(const double* const xyzs, std::size_t numPoints, bool ignoreZValues=false);
  smtk::mesh::PointLocatorImplPtr pointLocator(const float* const xyzs, std::size_t numPoints, bool ignoreZValues=false);

  //----------------------------------------------------------------------------
  smtk::mesh::Handle getRoot() const;

  //----------------------------------------------------------------------------
  //creates a mesh with that contains the input cells.
  //the mesh will have the root as its parent.
  //The mesh will be tagged with the GEOM_DIMENSION tag with a value that is
  //equal to highest dimension of cell inside
  //Will fail if the HandleRange is empty or doesn't contain valid
  //cell handles.
  //Note: Will mark the interface as modified when successful
  bool createMesh(const smtk::mesh::HandleRange& cells,
                  smtk::mesh::Handle& meshHandle);

  //----------------------------------------------------------------------------
  std::size_t numMeshes(smtk::mesh::Handle handle) const;

  //----------------------------------------------------------------------------
  smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle) const;

  //----------------------------------------------------------------------------
  smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle,
                                       int dimension) const;

  //----------------------------------------------------------------------------
  //find all entity sets that have this exact name tag
  smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle,
                                       const std::string& name) const;

  //----------------------------------------------------------------------------
  //find all entity sets that have this exact domain tag
  smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle,
                                       const smtk::mesh::Domain& domain) const;

  //----------------------------------------------------------------------------
  //find all entity sets that have this exact dirichlet tag
  smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle,
                                      const smtk::mesh::Dirichlet& dirichlet) const;

  //----------------------------------------------------------------------------
  //find all entity sets that have this exact neumann tag
  smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle,
                                      const smtk::mesh::Neumann& neumann) const;

  //----------------------------------------------------------------------------
  //get all cells held by this range
  smtk::mesh::HandleRange getCells(const smtk::mesh::HandleRange& meshsets) const;

  //----------------------------------------------------------------------------
  //get all cells held by this range handle of a given cell type
  smtk::mesh::HandleRange getCells(const smtk::mesh::HandleRange& meshsets,
                                    smtk::mesh::CellType cellType) const;

  //----------------------------------------------------------------------------
  //get all cells held by this range handle of a given cell type(s)
  smtk::mesh::HandleRange getCells(const smtk::mesh::HandleRange& meshsets,
                                    const smtk::mesh::CellTypes& cellTypes) const;

  //----------------------------------------------------------------------------
  //get all cells held by this range handle of a given dimension
  smtk::mesh::HandleRange getCells(const smtk::mesh::HandleRange& meshsets,
                                    smtk::mesh::DimensionType dim) const;

  //----------------------------------------------------------------------------
  //get all points held by this range of handle of a given dimension
  smtk::mesh::HandleRange getPoints(const smtk::mesh::HandleRange& cells) const;

  //----------------------------------------------------------------------------
  //get all the coordinates for the points in this range
  //xyz needs to be allocated to 3*points.size()
  //Floats are not how we store the coordinates internally, so asking for
  //the coordinates in such a manner could cause data inaccuracies to appear
  //so generally this is only used if you fully understand the input domain
  bool getCoordinates(const smtk::mesh::HandleRange& points,
                      double* xyz) const;

  //----------------------------------------------------------------------------
  //get all the coordinates for the points in this range
  //xyz needs to be allocated to 3*points.size()
  bool getCoordinates(const smtk::mesh::HandleRange& points,
                      float* xyz) const;

  //----------------------------------------------------------------------------
  //set all the coordinates for the points in this range
  //xyz needs to be allocated to 3*points.size()
  virtual bool setCoordinates(const smtk::mesh::HandleRange& points,
                              const double* const xyz);

  //----------------------------------------------------------------------------
  //set all the coordinates for the points in this range
  //xyz needs to be allocated to 3*points.size()
  virtual bool setCoordinates(const smtk::mesh::HandleRange& points,
                              const float* const xyz);

  //----------------------------------------------------------------------------
  std::vector< std::string > computeNames(const smtk::mesh::HandleRange& meshsets) const;

  //----------------------------------------------------------------------------
  std::vector< smtk::mesh::Domain > computeDomainValues(const smtk::mesh::HandleRange& meshsets) const;

  //----------------------------------------------------------------------------
  std::vector< smtk::mesh::Dirichlet > computeDirichletValues(const smtk::mesh::HandleRange& meshsets) const;

  //----------------------------------------------------------------------------
  std::vector< smtk::mesh::Neumann > computeNeumannValues(const smtk::mesh::HandleRange& meshsets) const;

  //----------------------------------------------------------------------------
  smtk::common::UUIDArray computeModelEntities(const smtk::mesh::HandleRange& meshsets) const;

  //----------------------------------------------------------------------------
  smtk::mesh::TypeSet computeTypes(const smtk::mesh::HandleRange& range) const;

  //----------------------------------------------------------------------------
  //compute the cells that make the shell/skin of the set of meshes
  bool computeShell(const smtk::mesh::HandleRange& meshes, smtk::mesh::HandleRange& shell) const;

  //----------------------------------------------------------------------------
  //merge any duplicate points used by the cells that have been passed
  //Note: Will mark the interface as modified when successful
  bool mergeCoincidentContactPoints(const smtk::mesh::HandleRange& meshes,
                                   double tolerance);

  //----------------------------------------------------------------------------
  bool setDomain(const smtk::mesh::HandleRange& meshsets,
                   const smtk::mesh::Domain& domain) const;

  //----------------------------------------------------------------------------
  bool setDirichlet(const smtk::mesh::HandleRange& meshsets,
                    const smtk::mesh::Dirichlet& dirichlet) const;

  //----------------------------------------------------------------------------
  bool setNeumann(const smtk::mesh::HandleRange& meshsets,
                  const smtk::mesh::Neumann& neumann) const;

  //----------------------------------------------------------------------------
  bool setAssociation(const smtk::common::UUID& modelUUID,
                      const smtk::mesh::HandleRange& range) const;

  //----------------------------------------------------------------------------
  smtk::mesh::HandleRange findAssociations(const smtk::mesh::Handle& root,
                                           const smtk::common::UUID& modelUUID) const;

  //----------------------------------------------------------------------------
  bool setRootAssociation(const smtk::common::UUID& modelUUID) const;

  //----------------------------------------------------------------------------
  smtk::common::UUID rootAssociation() const;

  //----------------------------------------------------------------------------
  smtk::mesh::HandleRange rangeIntersect(const smtk::mesh::HandleRange& a,
                                        const smtk::mesh::HandleRange& b) const;

  //----------------------------------------------------------------------------
  smtk::mesh::HandleRange rangeDifference(const smtk::mesh::HandleRange& a,
                                          const smtk::mesh::HandleRange& b) const;

  //----------------------------------------------------------------------------
  smtk::mesh::HandleRange rangeUnion(const smtk::mesh::HandleRange& a,
                                     const smtk::mesh::HandleRange& b) const;

  //----------------------------------------------------------------------------
  smtk::mesh::HandleRange pointIntersect(const smtk::mesh::HandleRange& a,
                                         const smtk::mesh::HandleRange& b,
                                         smtk::mesh::PointConnectivity& bpc,
                                         const smtk::mesh::ContainsFunctor& containsFunctor) const;
  //----------------------------------------------------------------------------
  smtk::mesh::HandleRange pointDifference(const smtk::mesh::HandleRange& a,
                                          const smtk::mesh::HandleRange& b,
                                          smtk::mesh::PointConnectivity& bpc,
                                          const smtk::mesh::ContainsFunctor& containsFunctor) const;

  //----------------------------------------------------------------------------
  void pointForEach( const HandleRange &points,
                     smtk::mesh::PointForEach& filter) const;

  //----------------------------------------------------------------------------
  void cellForEach( const HandleRange &cells,
                    smtk::mesh::PointConnectivity& pc,
                    smtk::mesh::CellForEach& filter) const;

  //----------------------------------------------------------------------------
  void meshForEach( const HandleRange &meshes,
                    smtk::mesh::MeshForEach& filter) const;

  //----------------------------------------------------------------------------
  bool deleteHandles(const smtk::mesh::HandleRange& toDel);

  //----------------------------------------------------------------------------
  ::moab::Interface * moabInterface() const;

  //----------------------------------------------------------------------------
  void setModifiedState(bool state) { m_modified = state; }

private:
  //holds a reference to the real moab interface
  smtk::shared_ptr< ::moab::Interface > m_iface;
  smtk::mesh::AllocatorPtr m_alloc;
  smtk::mesh::BufferedCellAllocatorPtr m_bcAlloc;
  mutable bool m_modified;
};

}
}
}

#endif
