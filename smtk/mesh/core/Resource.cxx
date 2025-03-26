//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/core/Component.h"

#include "smtk/mesh/core/queries/BoundingBox.h"

#include "smtk/mesh/moab/Interface.h"

#include "smtk/common/UUIDGenerator.h"
#include "smtk/model/EntityIterator.h"

namespace smtk
{
namespace mesh
{

class Resource::InternalImpl
{
public:
  InternalImpl()
    : Interface(smtk::mesh::moab::make_interface())
  {
  }

  InternalImpl(smtk::mesh::InterfacePtr interface)
    : Interface(interface)
  {
  }

  [[nodiscard]] const smtk::mesh::InterfacePtr& mesh_iface() const { return this->Interface; }

  [[nodiscard]] smtk::mesh::Handle mesh_root_handle() const { return this->Interface->getRoot(); }

private:
  smtk::mesh::InterfacePtr Interface;
};

namespace
{
typedef std::tuple<BoundingBox> QueryList;
}

Resource::Resource()
  : Superclass(smtk::common::UUIDGenerator::instance().random())
  , m_internals(new InternalImpl())
{
  queries().registerQueries<QueryList>();
  m_internals->mesh_iface()->registerQueries(*this);
}

Resource::Resource(const smtk::common::UUID& resourceID)
  : smtk::resource::DerivedFrom<Resource, smtk::geometry::Resource>(resourceID)
  , m_internals(new InternalImpl())
{
  queries().registerQueries<QueryList>();
  m_internals->mesh_iface()->registerQueries(*this);
}

Resource::Resource(smtk::mesh::InterfacePtr interface)
  : smtk::resource::DerivedFrom<Resource, smtk::geometry::Resource>(
      smtk::common::UUIDGenerator::instance().random())
  , m_internals(new InternalImpl(interface))
{
  queries().registerQueries<QueryList>();
  interface->registerQueries(*this);
}

Resource::Resource(const smtk::common::UUID& resourceID, smtk::mesh::InterfacePtr interface)
  : smtk::resource::DerivedFrom<Resource, smtk::geometry::Resource>(resourceID)
  , m_internals(new InternalImpl(interface))
{
  queries().registerQueries<QueryList>();
  interface->registerQueries(*this);
}

Resource::~Resource()
{
  // Our interface query caches should be flushed before our interface goes out
  // of scope.
  queries().caches().clear();

  delete m_internals;
}

smtk::resource::ComponentPtr Resource::find(const smtk::common::UUID& compId) const
{
  return std::static_pointer_cast<smtk::resource::Component>(
    Component::create(std::const_pointer_cast<smtk::mesh::Resource>(shared_from_this()), compId));
}

std::function<bool(const resource::Component&)> Resource::queryOperation(
  const std::string& queryString) const
{
  // TODO
  (void)queryString;
  return [](const resource::Component& /*unused*/) { return true; };
}

// visit all components in the resource.
void Resource::visit(smtk::resource::Component::Visitor& visitor) const
{
  class Visit : public smtk::mesh::MeshForEach
  {
  public:
    Visit(smtk::resource::Component::Visitor& visitor)
      : m_visitor(visitor)
    {
    }

    void forMesh(smtk::mesh::MeshSet& mesh) override
    {
      m_visitor(smtk::mesh::Component::create(mesh));
    }

  private:
    smtk::resource::Component::Visitor& m_visitor;
  };

  Visit visit_(visitor);

  smtk::mesh::for_each(this->meshes(), visit_);
}

const smtk::mesh::InterfacePtr& Resource::interface() const
{
  return m_internals->mesh_iface();
}

void Resource::swapInterfaces(smtk::mesh::ResourcePtr& other)
{
  smtk::mesh::Resource::InternalImpl* temp = other->m_internals;
  other->m_internals = m_internals;
  m_internals = temp;
}

bool Resource::isValid() const
{
  //make sure we have a valid uuid, and that our internals are valid
  return !this->id().isNull();
}

bool Resource::isModified() const
{
  //make sure we have a valid uuid, and that our internals are valid
  return this->interface()->isModified();
}

const smtk::common::FileLocation& Resource::readLocation() const
{
  return m_readLocation;
}

void Resource::readLocation(const smtk::common::FileLocation& n)
{
  m_readLocation = n;
  //if the write location hasn't been set, update it to be the read location
  if (m_writeLocation.empty())
  {
    m_writeLocation = n;
  }
}

const smtk::common::FileLocation& Resource::writeLocation() const
{
  return m_writeLocation;
}

void Resource::writeLocation(const smtk::common::FileLocation& n)
{
  m_writeLocation = n;
}

void Resource::clearReadWriteLocations()
{
  m_readLocation.clear();
  m_writeLocation.clear();
}

std::string Resource::interfaceName() const
{
  return this->interface()->name();
}

const smtk::common::UUID Resource::entity() const
{
  return this->id();
}

std::size_t Resource::numberOfMeshes() const
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  return iface->numMeshes(m_internals->mesh_root_handle());
}

smtk::mesh::TypeSet Resource::types() const
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  smtk::mesh::Handle handle = m_internals->mesh_root_handle();
  return iface->computeTypes(iface->getMeshsets(handle));
}

smtk::mesh::CellSet Resource::cells() const
{
  smtk::mesh::MeshSet ms(this->shared_from_this(), m_internals->mesh_root_handle());
  return ms.cells();
}

smtk::mesh::PointSet Resource::points() const
{
  smtk::mesh::MeshSet ms(this->shared_from_this(), m_internals->mesh_root_handle());
  return ms.points();
}

smtk::mesh::PointConnectivity Resource::pointConnectivity() const
{
  smtk::mesh::MeshSet ms(this->shared_from_this(), m_internals->mesh_root_handle());
  return ms.pointConnectivity();
}

void Resource::assignDefaultNames()
{
  smtk::resource::Component::Visitor nameAssigner =
    [this](const smtk::resource::Component::Ptr& comp) {
      auto mset = comp ? comp->as<smtk::mesh::Component>() : nullptr;
      if (!mset || !mset->name().empty())
      {
        return;
      }

      // Keep generating names until we find an unused one.
      std::string nameToTry;
      do
      {
        m_nameCounter++;
        std::ostringstream namer;
        namer << "mesh " << m_nameCounter;
        nameToTry = namer.str();
      } while (this->meshes(nameToTry).isValid());
      mset->mesh().setName(nameToTry);
    };
  this->visit(nameAssigner);
}

smtk::mesh::MeshSet Resource::meshes() const
{
  return smtk::mesh::MeshSet(this->shared_from_this(), m_internals->mesh_root_handle());
}

std::vector<std::string> Resource::meshNames() const
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  smtk::mesh::Handle handle = m_internals->mesh_root_handle();

  smtk::mesh::HandleRange entities = iface->getMeshsets(handle);
  return iface->computeNames(entities);
}

smtk::mesh::MeshSet Resource::meshes(smtk::mesh::DimensionType dim) const
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  smtk::mesh::Handle handle = m_internals->mesh_root_handle();
  const int dim_value = static_cast<int>(dim);

  smtk::mesh::HandleRange entities = iface->getMeshsets(handle, dim_value);
  return smtk::mesh::MeshSet(this->shared_from_this(), m_internals->mesh_root_handle(), entities);
}

smtk::mesh::MeshSet Resource::meshes(const std::string& name) const
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  smtk::mesh::Handle handle = m_internals->mesh_root_handle();

  smtk::mesh::HandleRange entities = iface->getMeshsets(handle, name);
  return smtk::mesh::MeshSet(this->shared_from_this(), m_internals->mesh_root_handle(), entities);
}

smtk::mesh::MeshSet Resource::meshes(const smtk::mesh::Domain& d) const
{
  return this->domainMeshes(d);
}

smtk::mesh::MeshSet Resource::meshes(const smtk::mesh::Dirichlet& d) const
{
  return this->dirichletMeshes(d);
}

smtk::mesh::MeshSet Resource::meshes(const smtk::mesh::Neumann& n) const
{
  return this->neumannMeshes(n);
}

smtk::mesh::CellSet Resource::cells(smtk::mesh::CellType cellType) const
{
  smtk::mesh::MeshSet ms(this->shared_from_this(), m_internals->mesh_root_handle());
  return ms.cells(cellType);
}

smtk::mesh::CellSet Resource::cells(smtk::mesh::CellTypes cellTypes) const
{
  smtk::mesh::MeshSet ms(this->shared_from_this(), m_internals->mesh_root_handle());
  return ms.cells(cellTypes);
}

smtk::mesh::CellSet Resource::cells(smtk::mesh::DimensionType dim) const
{
  smtk::mesh::MeshSet ms(this->shared_from_this(), m_internals->mesh_root_handle());
  return ms.cells(dim);
}

bool Resource::classifyTo(const smtk::model::ResourcePtr& resource)
{
  smtk::model::ResourcePtr currentResource = this->classifiedTo();
  if (currentResource != nullptr)
  {
    this->links().removeLinksTo(
      std::static_pointer_cast<smtk::resource::Resource>(currentResource), ClassificationRole);
  }
  return this->links()
           .addLinkTo(
             std::static_pointer_cast<smtk::resource::Resource>(resource), ClassificationRole)
           .first != smtk::common::UUID::null();
}

smtk::model::ResourcePtr Resource::classifiedTo() const
{
  auto classifiedObjects = this->links().linkedTo(ClassificationRole);
  return (
    !classifiedObjects.empty()
      ? std::dynamic_pointer_cast<smtk::model::Resource>(*classifiedObjects.begin())
      : smtk::model::ResourcePtr());
}

smtk::mesh::TypeSet Resource::findAssociatedTypes(const smtk::model::EntityRef& eref) const
{
  return this->findAssociatedMeshes(eref).types();
}

smtk::mesh::MeshSet Resource::findAssociatedMeshes(const smtk::model::EntityRef& eref) const
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  smtk::mesh::Handle handle = m_internals->mesh_root_handle();

  return smtk::mesh::MeshSet(
    this->shared_from_this(), handle, iface->findAssociations(handle, eref.entity()));
}

smtk::mesh::MeshSet Resource::findAssociatedMeshes(
  const smtk::model::EntityRef& eref,
  smtk::mesh::DimensionType dim) const
{
  smtk::mesh::MeshSet unfiltered = this->findAssociatedMeshes(eref);
  return unfiltered.subset(dim);
}

smtk::mesh::CellSet Resource::findAssociatedCells(const smtk::model::EntityRef& eref) const
{
  smtk::mesh::MeshSet ms = this->findAssociatedMeshes(eref);
  return ms.cells();
}

smtk::mesh::CellSet Resource::findAssociatedCells(
  const smtk::model::EntityRef& eref,
  smtk::mesh::CellType cellType) const
{
  smtk::mesh::MeshSet ms = this->findAssociatedMeshes(eref);
  return ms.cells(cellType);
}

smtk::mesh::CellSet Resource::findAssociatedCells(
  const smtk::model::EntityRef& eref,
  smtk::mesh::DimensionType dim) const
{
  smtk::mesh::MeshSet ms = this->findAssociatedMeshes(eref, dim);
  return ms.cells();
}

smtk::mesh::TypeSet Resource::findAssociatedTypes(const smtk::common::UUID& id) const
{
  return this->findAssociatedMeshes(id).types();
}

smtk::mesh::MeshSet Resource::findAssociatedMeshes(const smtk::common::UUID& id) const
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  smtk::mesh::Handle handle = m_internals->mesh_root_handle();

  return smtk::mesh::MeshSet(this->shared_from_this(), handle, iface->findAssociations(handle, id));
}

smtk::mesh::MeshSet Resource::findAssociatedMeshes(
  const smtk::common::UUID& id,
  smtk::mesh::DimensionType dim) const
{
  smtk::mesh::MeshSet unfiltered = this->findAssociatedMeshes(id);
  return unfiltered.subset(dim);
}

smtk::mesh::CellSet Resource::findAssociatedCells(const smtk::common::UUID& id) const
{
  smtk::mesh::MeshSet ms = this->findAssociatedMeshes(id);
  return ms.cells();
}

smtk::mesh::CellSet Resource::findAssociatedCells(
  const smtk::common::UUID& id,
  smtk::mesh::CellType cellType) const
{
  smtk::mesh::MeshSet ms = this->findAssociatedMeshes(id);
  return ms.cells(cellType);
}

smtk::mesh::CellSet Resource::findAssociatedCells(
  const smtk::common::UUID& id,
  smtk::mesh::DimensionType dim) const
{
  smtk::mesh::MeshSet ms = this->findAssociatedMeshes(id, dim);
  return ms.cells();
}

smtk::mesh::TypeSet Resource::findAssociatedTypes(smtk::model::EntityIterator& refIt) const
{
  return this->findAssociatedMeshes(refIt).types();
}

smtk::mesh::MeshSet Resource::findAssociatedMeshes(smtk::model::EntityIterator& refIt) const
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  smtk::mesh::Handle handle = m_internals->mesh_root_handle();

  smtk::mesh::HandleRange range;
  for (refIt.begin(); !refIt.isAtEnd(); ++refIt)
  {
    range = range | iface->findAssociations(handle, (*refIt).entity());
  }

  return smtk::mesh::MeshSet(this->shared_from_this(), handle, range);
}

smtk::mesh::MeshSet Resource::findAssociatedMeshes(
  smtk::model::EntityIterator& refIt,
  smtk::mesh::DimensionType dim) const
{
  smtk::mesh::MeshSet unfiltered = this->findAssociatedMeshes(refIt);
  return unfiltered.subset(dim);
}

smtk::mesh::CellSet Resource::findAssociatedCells(smtk::model::EntityIterator& refIt) const
{
  smtk::mesh::MeshSet ms = this->findAssociatedMeshes(refIt);
  return ms.cells();
}

smtk::mesh::CellSet Resource::findAssociatedCells(
  smtk::model::EntityIterator& refIt,
  smtk::mesh::CellType cellType) const
{
  smtk::mesh::MeshSet ms = this->findAssociatedMeshes(refIt);
  return ms.cells(cellType);
}

smtk::mesh::CellSet Resource::findAssociatedCells(
  smtk::model::EntityIterator& refIt,
  smtk::mesh::DimensionType dim) const
{
  smtk::mesh::MeshSet ms = this->findAssociatedMeshes(refIt, dim);
  return ms.cells();
}

bool Resource::setAssociation(
  const smtk::model::EntityRef& eref,
  const smtk::mesh::MeshSet& meshset)
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  // This causes the eref to become a meshset with the tag MODEL;
  // then all meshsets in m_range become child meshsets of eref:
  return iface->setAssociation(eref.entity(), meshset.m_range);
}

bool Resource::hasAssociations() const
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();

  smtk::mesh::Handle handle = m_internals->mesh_root_handle();
  smtk::mesh::HandleRange entities = iface->getMeshsets(handle);

  smtk::common::UUIDArray associations = iface->computeModelEntities(entities);
  return !associations.empty();
}

bool Resource::associateToModel(const smtk::common::UUID& uuid)
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();

  //before we associate this model make sure we are not already associated
  //to this model UUID. This is needed so that we are serializing/de-serializing
  //meshes we don't set the modify bit to true, when we re-associate to the model
  //and already had that info
  if (iface->rootAssociation() != uuid)
  {
    return iface->setRootAssociation(uuid);
  }
  return true;
}

bool Resource::isAssociatedToModel() const
{
  return this->associatedModel() != smtk::common::UUID::null();
}

smtk::common::UUID Resource::associatedModel() const
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  return iface->rootAssociation();
}

smtk::mesh::MeshSet Resource::createMesh(
  const smtk::mesh::CellSet& cells,
  const smtk::common::UUID& uuid)
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();

  smtk::mesh::HandleRange entities;
  if (cells.m_parent == this->shared_from_this())
  {
    smtk::mesh::Handle meshSetHandle;
    const bool meshCreated = iface->createMesh(cells.range(), meshSetHandle);
    if (meshCreated)
    {
      entities.insert(meshSetHandle);
      if (uuid)
      {
        iface->setId(meshSetHandle, uuid);
      }
      else
      {
        iface->setId(meshSetHandle, smtk::common::UUIDGenerator::instance().random());
      }
    }
  }
  return smtk::mesh::MeshSet(this->shared_from_this(), m_internals->mesh_root_handle(), entities);
}

bool Resource::removeMeshes(const smtk::mesh::MeshSet& meshesToDelete)
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  if (meshesToDelete.m_parent == this->shared_from_this())
  {
    //When deleting a MeshSet we need to find all cells that
    //are not used by any other mesh.

    //find all other meshes
    smtk::mesh::MeshSet all_OtherMeshes =
      smtk::mesh::set_difference(this->meshes(), meshesToDelete);

    //now find the cells that are only used by the mesh we are about to delete.
    smtk::mesh::CellSet cellsToDelete =
      smtk::mesh::set_difference(meshesToDelete.cells(), all_OtherMeshes.cells());
    //delete our mesh and cells that aren't used by any one else
    const bool deletedMeshes = iface->deleteHandles(meshesToDelete.m_range);
    const bool deletedCells = iface->deleteHandles(cellsToDelete.range());
    return deletedMeshes && deletedCells;
  }
  return false;
}

std::vector<smtk::mesh::Domain> Resource::domains() const
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  smtk::mesh::Handle handle = m_internals->mesh_root_handle();

  smtk::mesh::HandleRange entities = iface->getMeshsets(handle);
  return iface->computeDomainValues(entities);
}

smtk::mesh::MeshSet Resource::domainMeshes(const smtk::mesh::Domain& d) const
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  smtk::mesh::Handle handle = m_internals->mesh_root_handle();

  smtk::mesh::HandleRange entities = iface->getMeshsets(handle, d);
  return smtk::mesh::MeshSet(this->shared_from_this(), m_internals->mesh_root_handle(), entities);
}

bool Resource::setDomainOnMeshes(const smtk::mesh::MeshSet& meshes, const smtk::mesh::Domain& d)
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  if (meshes.m_parent == this->shared_from_this())
  {
    return iface->setDomain(meshes.m_range, d);
  }
  return false;
}

std::vector<smtk::mesh::Dirichlet> Resource::dirichlets() const
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  smtk::mesh::Handle handle = m_internals->mesh_root_handle();

  smtk::mesh::HandleRange entities = iface->getMeshsets(handle);
  return iface->computeDirichletValues(entities);
}

smtk::mesh::MeshSet Resource::dirichletMeshes(const smtk::mesh::Dirichlet& d) const
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  smtk::mesh::Handle handle = m_internals->mesh_root_handle();

  smtk::mesh::HandleRange entities = iface->getMeshsets(handle, d);
  return smtk::mesh::MeshSet(this->shared_from_this(), m_internals->mesh_root_handle(), entities);
}

bool Resource::setDirichletOnMeshes(
  const smtk::mesh::MeshSet& meshes,
  const smtk::mesh::Dirichlet& d)
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  if (meshes.m_parent == this->shared_from_this())
  {
    return iface->setDirichlet(meshes.m_range, d);
  }
  return false;
}

std::vector<smtk::mesh::Neumann> Resource::neumanns() const
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  smtk::mesh::Handle handle = m_internals->mesh_root_handle();

  smtk::mesh::HandleRange entities = iface->getMeshsets(handle);
  return iface->computeNeumannValues(entities);
}

smtk::mesh::MeshSet Resource::neumannMeshes(const smtk::mesh::Neumann& n) const
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  smtk::mesh::Handle handle = m_internals->mesh_root_handle();

  smtk::mesh::HandleRange entities = iface->getMeshsets(handle, n);
  return smtk::mesh::MeshSet(this->shared_from_this(), m_internals->mesh_root_handle(), entities);
}

bool Resource::setNeumannOnMeshes(const smtk::mesh::MeshSet& meshes, const smtk::mesh::Neumann& n)
{
  const smtk::mesh::InterfacePtr& iface = m_internals->mesh_iface();
  if (meshes.m_parent == this->shared_from_this())
  {
    return iface->setNeumann(meshes.m_range, n);
  }
  return false;
}
} // namespace mesh
} // namespace smtk
