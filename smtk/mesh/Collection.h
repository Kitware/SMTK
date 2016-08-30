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

#include "smtk/CoreExports.h"
#include "smtk/SharedFromThis.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/UUID.h"
#include "smtk/common/FileLocation.h"

#include "smtk/mesh/CellSet.h"
#include "smtk/mesh/PointConnectivity.h"
#include "smtk/mesh/Handle.h"
#include "smtk/mesh/Interface.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/PropertyData.h"
#include "smtk/mesh/QueryTypes.h"
#include "smtk/mesh/TypeSet.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include <vector>

namespace smtk {

  //forward declare friends
  namespace io { class ImportMesh; }
  namespace model { class EntityIterator; }
  namespace mesh {

//Flyweight interface around a moab database of meshes. When constructed
//becomes registered with a manager with a weak relationship.
class SMTKCORE_EXPORT Collection : public smtk::enable_shared_from_this<Collection>
{
  //default constructor generates an invalid collection
  Collection();

#ifndef SHIBOKEN_SKIP
  //Construct a valid collection that is associated with a manager
  //but has an empty interface that can be populated
  Collection(const smtk::common::UUID& collectionID,
             smtk::mesh::ManagerPtr mngr );

  //Construct a valid collection that has an associated interface
  //in the future we need a better way to make collections refer
  //to different mesh interfaces
  Collection( const smtk::common::UUID& collectionID,
              smtk::mesh::InterfacePtr interface,
              smtk::mesh::ManagerPtr mngr);
#endif

public:
  smtkTypeMacro(Collection);
  //construct an invalid collection
  smtkCreateMacro(Collection);

  ~Collection();

  //determine if the given Collection is valid and is properly associated
  //to a manager.
  bool isValid() const;

  //determine if the Collection has been modified. Being Modified means that
  //the version we would write out to disk would differ from the version that
  //we loaded from disk. If a collection started as in-memory it
  //is considered modified once it is not empty.
  //Every time the collection is saved to disk, the Modified flag will be
  //reset to false.
  bool isModified() const;

  //get the name of a mesh collection
  const std::string& name() const;
  void name(const std::string& n);

  //assign the collection a unique name, given the current manager.
  //Note:
  //If the current name is already unique, no change will happen but we will
  //return true.
  //If the collection has no manager, the current name is not changed, and
  //false is returned
  bool assignUniqueNameIfNotAlready();

  //get the file that this collection was created from
  //will return an empty FileLocation if this collection wasn't read from file
  const smtk::common::FileLocation& readLocation() const;

  //set the file that this collection should be saved to.
  //By default this is set to be the same as the readLocation()
  void writeLocation(const smtk::common::FileLocation& path);
  void writeLocation(const std::string& path)
  { this->writeLocation(smtk::common::FileLocation(path)); }
  const smtk::common::FileLocation& writeLocation() const;

  //clear both the read and write locations for the collection. This
  //is generally done when de-serializing a collection and the read and write
  //locations are going to be deleted by the calling code.
  void clearReadWriteLocations();

  //get a string the identifies the interface type of the collection
  // valid types are:
  // "moab"
  // "json"
  //Note: all names will be all lower-case
  std::string interfaceName() const;


  //fetch the entity id for this uuid
  const smtk::common::UUID entity() const;

  //re-parent the collection onto a new manager.
  bool reparent(smtk::mesh::ManagerPtr newParent);

  std::size_t numberOfMeshes() const;

  //----------------------------------------------------------------------------
  //Queries on the full Collection
  //----------------------------------------------------------------------------
  smtk::mesh::TypeSet   types() const;
  smtk::mesh::MeshSet   meshes( ) const; //all meshes
  smtk::mesh::CellSet   cells( ) const; //all cells
  smtk::mesh::PointSet  points( ) const; //all points

  //todo:
  //find all cells of a given dimension that are attached to ?
  //smtk::mesh::CellSet   connectivity( smtk::mesh::DimensionType dim );

  smtk::mesh::PointConnectivity pointConnectivity( ) const; //all point connectivity info for all cells

  //For any mesh set that has a name we return that name. It is possible
  //that the we have un-named mesh sets.
  std::vector< std::string > meshNames() const;

  //Find all meshes that have at least one cell of the given type.
  //This means that you can get back meshes of mixed dimension
  //type.
  smtk::mesh::MeshSet   meshes( smtk::mesh::DimensionType dim ) const;
  smtk::mesh::MeshSet   meshes( const smtk::mesh::Domain& d ) const;
  smtk::mesh::MeshSet   meshes( const smtk::mesh::Dirichlet& d ) const;
  smtk::mesh::MeshSet   meshes( const smtk::mesh::Neumann& n ) const;
  smtk::mesh::MeshSet   meshes( const std::string& name ) const;

  //find a cells of a given type or a collection of types
  smtk::mesh::CellSet   cells( smtk::mesh::CellType cellType ) const;
  smtk::mesh::CellSet   cells( smtk::mesh::CellTypes cellTypes ) const;
  smtk::mesh::CellSet   cells( smtk::mesh::DimensionType dim ) const;

  //----------------------------------------------------------------------------
  // Queries by a model Cursor
  //----------------------------------------------------------------------------
  smtk::mesh::TypeSet   findAssociatedTypes( const smtk::model::EntityRef& eref ) const;
  smtk::mesh::MeshSet   findAssociatedMeshes( const smtk::model::EntityRef& eref ) const;
  smtk::mesh::MeshSet   findAssociatedMeshes( const smtk::model::EntityRef& eref, smtk::mesh::DimensionType dim ) const;
  smtk::mesh::CellSet   findAssociatedCells( const smtk::model::EntityRef& eref ) const;
  smtk::mesh::CellSet   findAssociatedCells( const smtk::model::EntityRef& eref, smtk::mesh::CellType cellType ) const;
  smtk::mesh::CellSet   findAssociatedCells( const smtk::model::EntityRef& eref, smtk::mesh::DimensionType dim ) const;

  smtk::mesh::TypeSet   findAssociatedTypes( smtk::model::EntityIterator& refIt ) const;
  smtk::mesh::MeshSet   findAssociatedMeshes( smtk::model::EntityIterator& refIt ) const;
  smtk::mesh::MeshSet   findAssociatedMeshes( smtk::model::EntityIterator& refIt,
                                              smtk::mesh::DimensionType dim ) const;
  smtk::mesh::CellSet   findAssociatedCells( smtk::model::EntityIterator& refIt ) const;
  smtk::mesh::CellSet   findAssociatedCells( smtk::model::EntityIterator& refIt,
                                             smtk::mesh::CellType cellType ) const;
  smtk::mesh::CellSet   findAssociatedCells( smtk::model::EntityIterator& refIt,
                                             smtk::mesh::DimensionType dim ) const;

  bool setAssociation( const smtk::model::EntityRef& eref, const smtk::mesh::MeshSet& meshset );

  //determine if this collection has any associations to a model
  bool hasAssociations() const;

  // Associate a model to the collection.
  bool associateToModel(const smtk::common::UUID& uuid);

  // Find if the collection has an associated model
  bool isAssociatedToModel() const;

  // Return the uuid of the associated model
  smtk::common::UUID associatedModel() const;

  //----------------------------------------------------------------------------
  // Construction of new meshes
  //----------------------------------------------------------------------------
  //given a collection of existing cells make a new Mesh inside the underlying interface
  //Return that Mesh as a MeshSet with a size of 1. The CellSet could
  //be the result of appending/intersecting,difference of other CellSets.
  //Adding a CellSet that is part of a different collection will fail, and
  //we will return an empty MeshSet.
  //Asking to create a MeshSet from a CellSet that is empty will fail, and
  //we will return an empty MeshSet.
  smtk::mesh::MeshSet createMesh( const smtk::mesh::CellSet& cells );

  //----------------------------------------------------------------------------
  // Deletion of Items
  //----------------------------------------------------------------------------
  //given a collection of meshes this will delete all meshes and any cell or vert
  //that is not referenced by any other mesh
  //This will invalidate any smtk::mesh::MeshSet that contains a reference to
  //one of the meshes that has been deleted.
  bool removeMeshes( smtk::mesh::MeshSet& meshesToDelete );

  //----------------------------------------------------------------------------
  // Domain Queries
  //----------------------------------------------------------------------------
  //get all the current domains
  std::vector< smtk::mesh::Domain > domains() const;

  //get the meshes with a given domain value. If no meshes have
  //this domain value the result will be empty
  smtk::mesh::MeshSet domainMeshes( const smtk::mesh::Domain& m ) const;

  //Assign a given domain to a collection of meshes. Overwrites
  //any existing domain value
  bool setDomainOnMeshes(const smtk::mesh::MeshSet& meshes,
                         const smtk::mesh::Domain& m);

  //----------------------------------------------------------------------------
  // Dirichlet Queries
  //----------------------------------------------------------------------------
  //get all the current dirichlet on the points of the mesh
  std::vector< smtk::mesh::Dirichlet > dirichlets() const;

  //get the meshes with a given dirichlet value. If no meshes have
  //this dirichlet value the result will be empty.
  //Generally Dirichlet meshes only contain vertices
  smtk::mesh::MeshSet dirichletMeshes( const smtk::mesh::Dirichlet& d ) const;

  //Assign a given dirichlet to a collection of meshes. Overwrites
  //any existing dirichlet value
  //Generally Dirichlet meshes only contain vertices
  bool setDirichletOnMeshes(const smtk::mesh::MeshSet& meshes,
                            const smtk::mesh::Dirichlet& d);

  //----------------------------------------------------------------------------
  // Neumann Queries
  //----------------------------------------------------------------------------
  //get all the current dirichlet on the points of the mesh
  std::vector< smtk::mesh::Neumann > neumanns() const;

  //get the meshes with a given neumann value. If no meshes have
  //this material value the result will be empty.
  smtk::mesh::MeshSet neumannMeshes( const smtk::mesh::Neumann& n ) const;

  //Assign a given neumann to a collection of meshes. Overwrites
  //any existing neumann value
  //Generally Neumann meshes only contain vertices
  bool setNeumannOnMeshes(const smtk::mesh::MeshSet& meshes,
                          const smtk::mesh::Neumann& n);

  const smtk::mesh::InterfacePtr& interface() const;

  void setModelManager(smtk::model::ManagerPtr mgr) { this->m_modelManager = mgr; }
  smtk::model::ManagerPtr modelManager() const { return this->m_modelManager.lock(); }


  //----------------------------------------------------------------------------
  // Float, String, Integer properties for a meshset given its handle range.
  //----------------------------------------------------------------------------
  void setFloatProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName, smtk::model::Float propValue);
  void setFloatProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName, const smtk::model::FloatList& propValue);
  smtk::model::FloatList const& floatProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName) const;
  smtk::model::FloatList& floatProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName);
  bool hasFloatProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName) const;
  bool removeFloatProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName);

  void setStringProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName, const smtk::model::String& propValue);
  void setStringProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName, const smtk::model::StringList& propValue);
  smtk::model::StringList const& stringProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName) const;
  smtk::model::StringList& stringProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName);
  bool hasStringProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName) const;
  bool removeStringProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName);

  void setIntegerProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName, smtk::model::Integer propValue);
  void setIntegerProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName, const smtk::model::IntegerList& propValue);
  smtk::model::IntegerList const& integerProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName) const;
  smtk::model::IntegerList& integerProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName);
  bool hasIntegerProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName) const;
  bool removeIntegerProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName);
#ifndef SHIBOKEN_SKIP
  // For T = {MeshIntegerData, MeshFloatData, MeshStringData}:
  template<typename T> T* properties();
  // For T = {IntegerData, FloatData, StringData}:
  template<typename T> T* meshProperties(const smtk::mesh::MeshSet& meshset);
  template<typename T> bool removeProperty(const smtk::mesh::MeshSet& meshset, const std::string& name);
#endif // SHIBOKEN_SKIP

private:
  Collection( const Collection& other ); //blank since we are used by shared_ptr
  Collection& operator=( const Collection& other ); //blank since we are used by shared_ptr

  //Sets the location that this collection was loaded from
  void readLocation(const std::string& path)
  { this->readLocation(smtk::common::FileLocation(path)); }
  void readLocation(const smtk::common::FileLocation& path);

  //Swap the internal interfaces between this Collection and another Collection
  //this is how we can easily update a collection that has already been
  //loaded with a newer version from disk
  void swapInterfaces(smtk::mesh::CollectionPtr& other);

  friend class smtk::mesh::Manager;
  friend class smtk::io::ImportMesh;
  friend class smtk::io::ImportJSON;

  //called by the manager that manages this collection, means that somebody
  //has requested us to be removed from a collection
  void removeManagerConnection( );

  smtk::common::UUID m_entity;
  std::string m_name;
  smtk::common::FileLocation m_readLocation;
  smtk::common::FileLocation m_writeLocation;

  smtk::model::WeakManagerPtr m_modelManager;
  smtk::shared_ptr<MeshFloatData> m_floatData;
  smtk::shared_ptr<MeshStringData> m_stringData;
  smtk::shared_ptr<MeshIntegerData> m_integerData;

  //holds a reference to both the manager and the specific backend interface
  class InternalImpl;
  smtk::mesh::Collection::InternalImpl* m_internals;
};

  } // namespace mesh
} // namespace smtk

#endif  //__smtk_mesh_Collection_h
