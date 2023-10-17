//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_mesh_core_Resource_h
#define smtk_mesh_core_Resource_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "smtk/common/FileLocation.h"
#include "smtk/common/UUID.h"

#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/Handle.h"
#include "smtk/mesh/core/Interface.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/PointConnectivity.h"
#include "smtk/mesh/core/QueryTypes.h"
#include "smtk/mesh/core/TypeSet.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"

#include "smtk/geometry/Resource.h"

#include "smtk/resource/DerivedFrom.h"

#include <vector>

namespace smtk
{
//forward declare friends
namespace io
{
class ReadMesh;
}
namespace model
{
class EntityIterator;
}
namespace mesh
{

//Flyweight interface around a moab database of meshes.
class SMTKCORE_EXPORT Resource
  : public smtk::resource::DerivedFrom<Resource, smtk::geometry::Resource>
{
  //default constructor generates an invalid resource
  Resource();

  //Construct a resource with the default interface.
  Resource(const smtk::common::UUID& resourceID);

  //Construct a valid resource that has an associated interface
  Resource(smtk::mesh::InterfacePtr interface);

  //Construct a valid resource that has an associated interface
  Resource(const smtk::common::UUID& resourceID, smtk::mesh::InterfacePtr interface);

public:
  smtkTypeMacro(smtk::mesh::Resource);
  smtkSuperclassMacro(smtk::resource::DerivedFrom<Resource, smtk::geometry::Resource>);
  smtkSharedPtrCreateMacro(smtk::resource::PersistentObject);

  Resource(const Resource& other) = delete;
  Resource& operator=(const Resource& other) = delete;

  // typedef referring to the parent resource.
  typedef smtk::geometry::Resource ParentResource;

  /// A mesh resource may be classified to a model, indicating the mesh set
  /// discretizes the linked model component.
  static constexpr smtk::resource::Links::RoleType ClassificationRole = -3;

  static smtk::shared_ptr<Resource> create(const smtk::common::UUID& resourceID)
  {
    smtk::shared_ptr<smtk::resource::Resource> shared(new Resource(resourceID));
    return smtk::static_pointer_cast<Resource>(shared);
  }

  static smtk::shared_ptr<Resource> create(smtk::mesh::InterfacePtr interface)
  {
    smtk::shared_ptr<smtk::resource::Resource> shared(new Resource(interface));
    return smtk::static_pointer_cast<Resource>(shared);
  }

  static smtk::shared_ptr<Resource> create(
    const smtk::common::UUID& resourceID,
    smtk::mesh::InterfacePtr interface)
  {
    smtk::shared_ptr<smtk::resource::Resource> shared(new Resource(resourceID, interface));
    return smtk::static_pointer_cast<Resource>(shared);
  }

  ~Resource() override;

  resource::ComponentPtr find(const common::UUID& compId) const override;
  std::function<bool(const resource::Component&)> queryOperation(const std::string&) const override;

  // visit all components in a resource.
  void visit(resource::Component::Visitor& v) const override;

  //determine if the given Resource is valid.
  bool isValid() const;

  //determine if the Resource has been modified. Being Modified means that
  //the version we would write out to disk would differ from the version that
  //we loaded from disk. If a resource started as in-memory it
  //is considered modified once it is not empty.
  //Every time the resource is saved to disk, the Modified flag will be
  //reset to false.
  bool isModified() const;

  //get the file that this resource was created from
  //will return an empty FileLocation if this resource wasn't read from file
  const smtk::common::FileLocation& readLocation() const;

  //set the file that this resource should be saved to.
  //By default this is set to be the same as the readLocation()
  void writeLocation(const smtk::common::FileLocation& path);
  void writeLocation(const std::string& path)
  {
    this->writeLocation(smtk::common::FileLocation(path));
  }
  const smtk::common::FileLocation& writeLocation() const;

  //clear both the read and write locations for the resource. This
  //is generally done when de-serializing a resource and the read and write
  //locations are going to be deleted by the calling code.
  void clearReadWriteLocations();

  //get a string the identifies the interface type of the resource
  // valid types are:
  // "moab"
  // "json"
  //Note: all names will be all lower-case
  std::string interfaceName() const;

  //fetch the entity id for this uuid
  const smtk::common::UUID entity() const;

  std::size_t numberOfMeshes() const;

  //Queries on the full Resource
  smtk::mesh::TypeSet types() const;
  smtk::mesh::MeshSet meshes() const;  //all meshes
  smtk::mesh::CellSet cells() const;   //all cells
  smtk::mesh::PointSet points() const; //all points

  //todo:
  //find all cells of a given dimension that are attached to ?
  //smtk::mesh::CellSet   connectivity( smtk::mesh::DimensionType dim );

  smtk::mesh::PointConnectivity pointConnectivity()
    const; //all point connectivity info for all cells

  /// Assign a machine-generated name to any mesh component that does not have a user-assigned name.
  void assignDefaultNames();

  //For any mesh set that has a name we return that name. It is possible
  //that the we have un-named mesh sets.
  std::vector<std::string> meshNames() const;

  //Find all meshes that have at least one cell of the given type.
  //This means that you can get back meshes of mixed dimension
  //type.
  smtk::mesh::MeshSet meshes(smtk::mesh::DimensionType dim) const;
  smtk::mesh::MeshSet meshes(const smtk::mesh::Domain& d) const;
  smtk::mesh::MeshSet meshes(const smtk::mesh::Dirichlet& d) const;
  smtk::mesh::MeshSet meshes(const smtk::mesh::Neumann& n) const;
  smtk::mesh::MeshSet meshes(const std::string& name) const;

  //find a cells of a given type or a resource of types
  smtk::mesh::CellSet cells(smtk::mesh::CellType cellType) const;
  smtk::mesh::CellSet cells(smtk::mesh::CellTypes cellTypes) const;
  smtk::mesh::CellSet cells(smtk::mesh::DimensionType dim) const;

  bool classifyTo(const smtk::model::ResourcePtr&);
  smtk::model::ResourcePtr classifiedTo() const;

  // Queries by a model Cursor
  smtk::mesh::TypeSet findAssociatedTypes(const smtk::model::EntityRef& eref) const;
  smtk::mesh::MeshSet findAssociatedMeshes(const smtk::model::EntityRef& eref) const;
  smtk::mesh::MeshSet findAssociatedMeshes(
    const smtk::model::EntityRef& eref,
    smtk::mesh::DimensionType dim) const;
  smtk::mesh::CellSet findAssociatedCells(const smtk::model::EntityRef& eref) const;
  smtk::mesh::CellSet findAssociatedCells(
    const smtk::model::EntityRef& eref,
    smtk::mesh::CellType cellType) const;
  smtk::mesh::CellSet findAssociatedCells(
    const smtk::model::EntityRef& eref,
    smtk::mesh::DimensionType dim) const;

  smtk::mesh::TypeSet findAssociatedTypes(const smtk::common::UUID& id) const;
  smtk::mesh::MeshSet findAssociatedMeshes(const smtk::common::UUID& id) const;
  smtk::mesh::MeshSet findAssociatedMeshes(
    const smtk::common::UUID& id,
    smtk::mesh::DimensionType dim) const;
  smtk::mesh::CellSet findAssociatedCells(const smtk::common::UUID& id) const;
  smtk::mesh::CellSet findAssociatedCells(
    const smtk::common::UUID& id,
    smtk::mesh::CellType cellType) const;
  smtk::mesh::CellSet findAssociatedCells(
    const smtk::common::UUID& id,
    smtk::mesh::DimensionType dim) const;

  smtk::mesh::TypeSet findAssociatedTypes(smtk::model::EntityIterator& refIt) const;
  smtk::mesh::MeshSet findAssociatedMeshes(smtk::model::EntityIterator& refIt) const;
  smtk::mesh::MeshSet findAssociatedMeshes(
    smtk::model::EntityIterator& refIt,
    smtk::mesh::DimensionType dim) const;
  smtk::mesh::CellSet findAssociatedCells(smtk::model::EntityIterator& refIt) const;
  smtk::mesh::CellSet findAssociatedCells(
    smtk::model::EntityIterator& refIt,
    smtk::mesh::CellType cellType) const;
  smtk::mesh::CellSet findAssociatedCells(
    smtk::model::EntityIterator& refIt,
    smtk::mesh::DimensionType dim) const;

  bool setAssociation(const smtk::model::EntityRef& eref, const smtk::mesh::MeshSet& meshset);

  //determine if this resource has any associations to a model
  bool hasAssociations() const;

  // Associate a model to the resource.
  //While a resource can be associated to just a model UUID, it is necessary
  //to also call setModelResource() to facilitate calls that return associated
  //EntityRefs, rather than just UUIDs.
  bool associateToModel(const smtk::common::UUID& uuid);

  // Find if the resource has an associated model
  bool isAssociatedToModel() const;

  // Return the uuid of the associated model
  smtk::common::UUID associatedModel() const;

  // Construction of new meshes
  //given a resource of existing cells make a new Mesh inside the underlying interface
  //Return that Mesh as a MeshSet with a size of 1. The CellSet could
  //be the result of appending/intersecting,difference of other CellSets.
  //Adding a CellSet that is part of a different resource will fail, and
  //we will return an empty MeshSet.
  //Asking to create a MeshSet from a CellSet that is empty will fail, and
  //we will return an empty MeshSet.
  smtk::mesh::MeshSet createMesh(
    const smtk::mesh::CellSet& cells,
    const smtk::common::UUID& uuid = smtk::common::UUID::null());

  // Deletion of Items
  //given a resource of meshes this will delete all meshes and any cell or vert
  //that is not referenced by any other mesh
  //This will invalidate any smtk::mesh::MeshSet that contains a reference to
  //one of the meshes that has been deleted.
  bool removeMeshes(const smtk::mesh::MeshSet& meshesToDelete);

  // Domain Queries
  //get all the current domains
  std::vector<smtk::mesh::Domain> domains() const;

  //get the meshes with a given domain value. If no meshes have
  //this domain value the result will be empty
  smtk::mesh::MeshSet domainMeshes(const smtk::mesh::Domain& m) const;

  //Assign a given domain to a resource of meshes. Overwrites
  //any existing domain value
  bool setDomainOnMeshes(const smtk::mesh::MeshSet& meshes, const smtk::mesh::Domain& m);

  // Dirichlet Queries
  //get all the current dirichlet on the points of the mesh
  std::vector<smtk::mesh::Dirichlet> dirichlets() const;

  //get the meshes with a given dirichlet value. If no meshes have
  //this dirichlet value the result will be empty.
  //Generally Dirichlet meshes only contain vertices
  smtk::mesh::MeshSet dirichletMeshes(const smtk::mesh::Dirichlet& d) const;

  //Assign a given dirichlet to a resource of meshes. Overwrites
  //any existing dirichlet value
  //Generally Dirichlet meshes only contain vertices
  bool setDirichletOnMeshes(const smtk::mesh::MeshSet& meshes, const smtk::mesh::Dirichlet& d);

  // Neumann Queries
  //get all the current dirichlet on the points of the mesh
  std::vector<smtk::mesh::Neumann> neumanns() const;

  //get the meshes with a given neumann value. If no meshes have
  //this material value the result will be empty.
  smtk::mesh::MeshSet neumannMeshes(const smtk::mesh::Neumann& n) const;

  //Assign a given neumann to a resource of meshes. Overwrites
  //any existing neumann value
  //Generally Neumann meshes only contain vertices
  bool setNeumannOnMeshes(const smtk::mesh::MeshSet& meshes, const smtk::mesh::Neumann& n);

  const smtk::mesh::InterfacePtr& interface() const;

  void setModelResource(smtk::model::ResourcePtr resource) { m_modelResource = resource; }
  smtk::model::ResourcePtr modelResource() const { return m_modelResource.lock(); }

private:
  //Sets the location that this resource was loaded from
  void readLocation(const std::string& path)
  {
    this->readLocation(smtk::common::FileLocation(path));
  }
  void readLocation(const smtk::common::FileLocation& path);

  //Swap the internal interfaces between this Resource and another Resource
  //this is how we can easily update a resource that has already been
  //loaded with a newer version from disk
  void swapInterfaces(smtk::mesh::ResourcePtr& other);

  friend class smtk::io::ReadMesh;

  smtk::common::FileLocation m_readLocation;
  smtk::common::FileLocation m_writeLocation;

  smtk::model::WeakResourcePtr m_modelResource;

  int m_nameCounter{ -1 };

  friend std::shared_ptr<Component> Component::create(
    const ResourcePtr&,
    const smtk::common::UUID&);
  friend std::shared_ptr<Component> Component::create(const MeshSet&);
  std::map<smtk::common::UUID, Component::Ptr> m_componentMap;

  //holds a reference to the specific backend interface
  class InternalImpl;
  smtk::mesh::Resource::InternalImpl* m_internals;
};

} // namespace mesh
} // namespace smtk

#endif // smtk_mesh_core_Resource_h
