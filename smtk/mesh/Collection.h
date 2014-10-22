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
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/UUID.h"
#include "smtk/mesh/QueryTypes.h"

#include "smtk/model/EntityRef.h"

namespace smtk {
namespace mesh {

//Flyweight interface around a moab database of meshes. When constructed
//becomes registered with a manager with a weak relationship.
class SMTKCORE_EXPORT Collection
{
public:

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

  ~Collection();

  //determine if the given Collection is valid and is properly associated
  //to a manager.
  bool isValid() const;

  //get the name of a mesh collection
  const std::string& name() const;

  //fetch the entity id for this uuid
  const smtk::common::UUID entity() const;

  std::size_t numberOfMeshes() const;

  //re-parent the collection onto a new manager.
  bool reparent(smtk::mesh::ManagerPtr newParent);

  //General Queries on a Collection
  smtk::mesh::TypeSet   associatedTypes( );
  smtk::mesh::CellSet   cells( );
  smtk::mesh::PointSet  points( );
  smtk::mesh::MeshSet   meshes( );

  //Advanced Queries on a Collection
  //find a cells of a given type or a collection of types
  smtk::mesh::CellSet   cells( smtk::mesh::CellType cellType );
  smtk::mesh::CellSet   cells( smtk::mesh::CellTypes cellTypes );

  smtk::mesh::PointSet  pointsAssociatedTo( smtk::mesh::CellType cellType );
  smtk::mesh::PointSet  pointsAssociatedTo( smtk::mesh::CellTypes cellTypes );


  //Query a Collection given a model cursor
  smtk::mesh::TypeSet   findAssociatedTypes( const smtk::model::EntityRef& eref );
  smtk::mesh::CellSet   findAssociatedCells( const smtk::model::EntityRef& eref );
  smtk::mesh::PointSet  findAssociatedPoints( const smtk::model::EntityRef& eref );
  smtk::mesh::MeshSet   findAssociatedMeshes( const smtk::model::EntityRef& eref );

  //Query a Collection given a model cursor and some QueryType interface ( cell, dim )
  smtk::mesh::CellSet   findAssociatedCells( const smtk::model::EntityRef& eref, smtk::mesh::CellType cellType );
  //The meshes returned can include mixed cell meshes so watch out
  smtk::mesh::MeshSet   findAssociatedMeshes( const smtk::model::EntityRef& eref, smtk::mesh::CellType cellType );
  smtk::mesh::MeshSet   findAssociatedMeshes( const smtk::model::EntityRef& eref, smtk::mesh::DimensionType dim );

private:
  friend class smtk::mesh::Manager;

  //called by the manager that manages this collection, means that somebody
  //has requested us to be removed from a collection
  void removeManagerConnection( );

  smtk::common::UUID m_entity;
  std::string m_name;

  //holds a reference to both the manager and the moab interface
  //in the future this should be switchable to allow different interface
  //types
  class InternalImpl;
  smtk::shared_ptr< InternalImpl > m_internals;
};

}
}

#endif  //__smtk_mesh_Collection_h