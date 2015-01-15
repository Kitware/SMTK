//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_Collection_h
#define __smtk_mesh_Collection_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/SharedFromThis.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/UUID.h"

#include "smtk/mesh/CellSet.h"
#include "smtk/mesh/PointConnectivity.h"
#include "smtk/mesh/Handle.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/QueryTypes.h"
#include "smtk/mesh/TypeSet.h"

#include "smtk/model/EntityRef.h"

#include <vector>

namespace smtk {
namespace mesh {

//Flyweight interface around a moab database of meshes. When constructed
//becomes registered with a manager with a weak relationship.
class SMTKCORE_EXPORT Collection : public smtk::enable_shared_from_this<Collection>
{
  //default constructor generates an invalid collection
  Collection();

  //Construct a valid collection that is associated with a manager
  //but has an empty interface that can be populated
  Collection( smtk::mesh::ManagerPtr mngr );

  //Construct a valid collection that has an associated interface
  //in the future we need a better way to make collections refer
  //to different mesh interfaces
  Collection( smtk::mesh::moab::InterfacePtr interface,
              smtk::mesh::ManagerPtr mngr);

public:
  smtkTypeMacro(Collection);
  //construct an invalid collection
  smtkCreateMacro(Collection);

  ~Collection();

  //determine if the given Collection is valid and is properly associated
  //to a manager.
  bool isValid() const;

  //get the name of a mesh collection
  const std::string& name() const;

  //fetch the entity id for this uuid
  const smtk::common::UUID entity() const;

  //re-parent the collection onto a new manager.
  bool reparent(smtk::mesh::ManagerPtr newParent);

  std::size_t numberOfMeshes() const;

  //----------------------------------------------------------------------------
  //Queries on the full Collection
  //----------------------------------------------------------------------------
  smtk::mesh::TypeSet   associatedTypes( ) const;
  smtk::mesh::MeshSet   meshes( ); //all meshes
  smtk::mesh::CellSet   cells( ); //all cells
  smtk::mesh::Points    points( ); //all points
  smtk::mesh::PointConnectivity pointConnectivity( ); //all point connectivity info for all cells

  //For any mesh set that has a name we return that name. It is possible
  //that the we have un-named mesh sets.
  std::vector< std::string > meshNames();

  //all meshes of that are labeled of a given dimension, which generally
  //is the high dimension inside that mesh
  smtk::mesh::MeshSet   meshes( smtk::mesh::DimensionType dim );
  smtk::mesh::MeshSet   meshes( const std::string& name );

  //find a cells of a given type or a collection of types
  smtk::mesh::CellSet   cells( smtk::mesh::CellType cellType );
  smtk::mesh::CellSet   cells( smtk::mesh::CellTypes cellTypes );
  smtk::mesh::CellSet   cells( smtk::mesh::DimensionType dim );

  //----------------------------------------------------------------------------
  // Queries by a model Cursor
  //----------------------------------------------------------------------------
  smtk::mesh::TypeSet   findAssociatedTypes( const smtk::model::EntityRef& eref );
  smtk::mesh::MeshSet   findAssociatedMeshes( const smtk::model::EntityRef& eref );
  smtk::mesh::MeshSet   findAssociatedMeshes( const smtk::model::EntityRef& eref, smtk::mesh::CellType cellType );
  smtk::mesh::MeshSet   findAssociatedMeshes( const smtk::model::EntityRef& eref, smtk::mesh::DimensionType dim );
  smtk::mesh::CellSet   findAssociatedCells( const smtk::model::EntityRef& eref );
  smtk::mesh::CellSet   findAssociatedCells( const smtk::model::EntityRef& eref, smtk::mesh::CellType cellType );
  smtk::mesh::CellSet   findAssociatedCells( const smtk::model::EntityRef& eref, smtk::mesh::DimensionType dim );

  //todo: query based on boundary and other attributes of the mesh db
  //Tag("BOUNDARY_SET"){};
  //smtk::mesh:::MeshSet bodunaryMeshes();
  //Tag("DIRICHLET_SET"){};
  //smtk::mesh:::MeshSet dirichletMeshes();
  //Tag("NEUMANN_SET"){};
  //smtk::mesh:::MeshSet neumannMeshes();
  //todo: need to be able to extract the entire surface of the mesh
  //smtk::mesh::MeshSet generateBoundarMeshes();

private:
  Collection( const Collection& other ); //blank since we are used by shared_ptr
  Collection& operator=( const Collection& other ); //blank since we are used by shared_ptr

  friend class smtk::mesh::Manager;
  friend const smtk::mesh::moab::InterfacePtr& smtk::mesh::moab::extractInterface(smtk::mesh::CollectionPtr c);

  //called by friend extractInterface to get the interface shared_ptr
  //that we hold
  const smtk::mesh::moab::InterfacePtr& extractInterface() const;

  //called by the manager that manages this collection, means that somebody
  //has requested us to be removed from a collection
  void removeManagerConnection( );

  smtk::common::UUID m_entity;
  std::string m_name;

  //holds a reference to both the manager and the moab interface
  //in the future this should be switchable to allow different interface
  //types. Using a raw pointer while we decide if we can bring in
  //scoped_ptr / unique_ptr.
  class InternalImpl;
  smtk::mesh::Collection::InternalImpl* m_internals;
};

}
}

#endif  //__smtk_mesh_Collection_h