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

#include "smtk/SMTKCoreExports.h"
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
::moab::Interface* const extract_moab_interface( const smtk::mesh::InterfacePtr &iface);

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT Interface : public smtk::mesh::Interface
{
public:
  Interface();

  virtual ~Interface();

  //----------------------------------------------------------------------------
  //get back a lightweight interface around allocating memory into the given
  //interface. This is generally used to create new coordinates or cells that
  //are than assigned to an existing mesh or new mesh
  smtk::mesh::AllocatorPtr allocator();

  //----------------------------------------------------------------------------
  smtk::mesh::Handle getRoot() const;

  //----------------------------------------------------------------------------
  //creates a mesh with that contains the input cells.
  //the mesh will have the root as its parent.
  //The mesh will be tagged with the GEOM_DIMENSION tag with a value that is
  //equal to highest dimension of cell inside
  //Will fail if the HandleRange is empty or doesn't contain valid
  //cell handles.
  virtual bool createMesh(const smtk::mesh::HandleRange& cells,
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
  //find all entity sets that have this exact material tag
  smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle,
                                       const smtk::mesh::Material& material) const;

  //----------------------------------------------------------------------------
  //find all entity sets that have this exact dirichlet tag
  smtk::mesh::HandleRange getMeshsets(smtk::mesh::Handle handle,
                                      const smtk::mesh::Dirichlet& dirichlet) const;

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
  std::vector< std::string > computeNames(const smtk::mesh::HandleRange& meshsets) const;

  //----------------------------------------------------------------------------
  std::vector< smtk::mesh::Material > computeMaterialValues(const smtk::mesh::HandleRange& meshsets) const;

  //----------------------------------------------------------------------------
  std::vector< smtk::mesh::Dirichlet > computeDirichletValues(const smtk::mesh::HandleRange& meshsets) const;

  //----------------------------------------------------------------------------
  smtk::mesh::TypeSet computeTypes(smtk::mesh::Handle handle) const;

  //----------------------------------------------------------------------------
  //compute the cells that make the shell/skin of the set of meshes
  bool computeShell(const smtk::mesh::HandleRange& meshes, smtk::mesh::HandleRange& shell) const;

  //----------------------------------------------------------------------------
  bool setMaterial(const smtk::mesh::HandleRange& meshsets,
                   const smtk::mesh::Material& material) const;

  //----------------------------------------------------------------------------
  bool setDirichlet(const smtk::mesh::HandleRange& meshsets,
                    const smtk::mesh::Dirichlet& dirichlet) const;

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
  bool deleteHandles(const smtk::mesh::HandleRange& toDel);

  //----------------------------------------------------------------------------
  ::moab::Interface * const moabInterface() const;

private:
  //holds a reference to the real moab interface
  smtk::shared_ptr< ::moab::Interface > m_iface;
  smtk::mesh::AllocatorPtr m_alloc;
};

}
}
}

#endif
