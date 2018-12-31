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

#include "smtk/mesh/moab/Interface.h"

#include "smtk/common/UUIDGenerator.h"
#include "smtk/model/EntityIterator.h"

namespace smtk
{
namespace mesh
{

constexpr smtk::resource::Links::RoleType Resource::ClassificationRole;

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

  const smtk::mesh::InterfacePtr& mesh_iface() const { return this->Interface; }

  smtk::mesh::Handle mesh_root_handle() const { return this->Interface->getRoot(); }

private:
  smtk::mesh::InterfacePtr Interface;
};

Resource::Resource()
  : smtk::resource::DerivedFrom<Resource, smtk::resource::Resource>(
      smtk::common::UUIDGenerator::instance().random())
  , m_name()
  , m_readLocation()
  , m_writeLocation()
  , m_floatData(new MeshFloatData)
  , m_stringData(new MeshStringData)
  , m_integerData(new MeshIntegerData)
  , m_nameCounter(-1)
  , m_internals(new InternalImpl())
{
}

Resource::Resource(const smtk::common::UUID& resourceID)
  : smtk::resource::DerivedFrom<Resource, smtk::resource::Resource>(resourceID)
  , m_name()
  , m_readLocation()
  , m_writeLocation()
  , m_floatData(new MeshFloatData)
  , m_stringData(new MeshStringData)
  , m_integerData(new MeshIntegerData)
  , m_nameCounter(-1)
  , m_internals(new InternalImpl())
{
}

Resource::Resource(smtk::mesh::InterfacePtr interface)
  : smtk::resource::DerivedFrom<Resource, smtk::resource::Resource>(
      smtk::common::UUIDGenerator::instance().random())
  , m_name()
  , m_readLocation()
  , m_writeLocation()
  , m_floatData(new MeshFloatData)
  , m_stringData(new MeshStringData)
  , m_integerData(new MeshIntegerData)
  , m_nameCounter(-1)
  , m_internals(new InternalImpl(interface))
{
}

Resource::Resource(const smtk::common::UUID& resourceID, smtk::mesh::InterfacePtr interface)
  : smtk::resource::DerivedFrom<Resource, smtk::resource::Resource>(resourceID)
  , m_name()
  , m_readLocation()
  , m_writeLocation()
  , m_floatData(new MeshFloatData)
  , m_stringData(new MeshStringData)
  , m_integerData(new MeshIntegerData)
  , m_nameCounter(-1)
  , m_internals(new InternalImpl(interface))
{
}

Resource::~Resource()
{
  if (m_internals)
  {
    delete m_internals;
  }
}

smtk::resource::ComponentPtr Resource::find(const smtk::common::UUID& compId) const
{
  return std::static_pointer_cast<smtk::resource::Component>(
    Component::create(std::const_pointer_cast<smtk::mesh::Resource>(shared_from_this()), compId));
}

std::function<bool(const resource::ConstComponentPtr&)> Resource::queryOperation(
  const std::string& queryString) const
{
  // TODO
  (void)queryString;
  return [](const resource::ConstComponentPtr&) { return true; };
}

// visit all components in the resource.
void Resource::visit(smtk::resource::Component::Visitor& visitor) const
{
  class Visit : public smtk::mesh::MeshForEach
  {
  public:
    Visit(smtk::resource::Component::Visitor& visitor)
      : smtk::mesh::MeshForEach()
      , m_visitor(visitor)
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
  return (this->id().isNull() != true);
}

bool Resource::isModified() const
{
  //make sure we have a valid uuid, and that our internals are valid
  return this->interface()->isModified();
}

std::string Resource::name() const
{
  return m_name;
}

void Resource::name(const std::string& n)
{
  m_name = n;
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
  smtk::resource::Component::Visitor nameAssigner = [this](
    const smtk::resource::Component::Ptr& comp) {
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
  return (!classifiedObjects.empty()
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
  const smtk::model::EntityRef& eref, smtk::mesh::DimensionType dim) const
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
  const smtk::model::EntityRef& eref, smtk::mesh::CellType cellType) const
{
  smtk::mesh::MeshSet ms = this->findAssociatedMeshes(eref);
  return ms.cells(cellType);
}

smtk::mesh::CellSet Resource::findAssociatedCells(
  const smtk::model::EntityRef& eref, smtk::mesh::DimensionType dim) const
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
  const smtk::common::UUID& id, smtk::mesh::DimensionType dim) const
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
  const smtk::common::UUID& id, smtk::mesh::CellType cellType) const
{
  smtk::mesh::MeshSet ms = this->findAssociatedMeshes(id);
  return ms.cells(cellType);
}

smtk::mesh::CellSet Resource::findAssociatedCells(
  const smtk::common::UUID& id, smtk::mesh::DimensionType dim) const
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
  smtk::model::EntityIterator& refIt, smtk::mesh::DimensionType dim) const
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
  smtk::model::EntityIterator& refIt, smtk::mesh::CellType cellType) const
{
  smtk::mesh::MeshSet ms = this->findAssociatedMeshes(refIt);
  return ms.cells(cellType);
}

smtk::mesh::CellSet Resource::findAssociatedCells(
  smtk::model::EntityIterator& refIt, smtk::mesh::DimensionType dim) const
{
  smtk::mesh::MeshSet ms = this->findAssociatedMeshes(refIt, dim);
  return ms.cells();
}

bool Resource::setAssociation(
  const smtk::model::EntityRef& eref, const smtk::mesh::MeshSet& meshset)
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
  const smtk::mesh::CellSet& cells, const smtk::common::UUID& uuid)
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
        iface->setId(meshCreated, uuid);
      }
      else
      {
        iface->setId(meshCreated, smtk::common::UUIDGenerator::instance().random());
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
  const smtk::mesh::MeshSet& meshes, const smtk::mesh::Dirichlet& d)
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

/** @name Model property accessors.
  *
  */
///@{
void Resource::setFloatProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName, smtk::model::Float propValue)
{
  smtk::model::FloatList tmp;
  tmp.push_back(propValue);
  this->setFloatProperty(meshset, propName, tmp);
}

void Resource::setFloatProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName,
  const smtk::model::FloatList& propValue)
{
  if (meshset.size() > 0)
  {
    (*m_floatData)[meshset][propName] = propValue;
  }
}

smtk::model::FloatList const& Resource::floatProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName) const
{
  if (meshset.size() > 0)
  {
    smtk::model::FloatData& floats((*m_floatData)[meshset]);
    return floats[propName];
  }
  static smtk::model::FloatList dummy;
  return dummy;
}

smtk::model::FloatList& Resource::floatProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName)
{
  if (meshset.size() > 0)
  {
    smtk::model::FloatData& floats((*m_floatData)[meshset]);
    return floats[propName];
  }
  static smtk::model::FloatList dummy;
  return dummy;
}

bool Resource::hasFloatProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName) const
{
  smtk::mesh::MeshFloatData::const_iterator uit = m_floatData->find(meshset);
  if (uit == m_floatData->end())
  {
    return false;
  }
  smtk::model::FloatData::const_iterator sit = uit->second.find(propName);
  // FIXME: Should we return true even when the array (*sit) is empty?
  return sit == uit->second.end() ? false : true;
}

bool Resource::removeFloatProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName)
{
  smtk::mesh::MeshFloatData::iterator uit = m_floatData->find(meshset);
  if (uit == m_floatData->end())
  {
    return false;
  }
  smtk::model::FloatData::iterator sit = uit->second.find(propName);
  if (sit == uit->second.end())
  {
    return false;
  }
  uit->second.erase(sit);
  if (uit->second.empty())
    m_floatData->erase(uit);
  return true;
}

void Resource::setStringProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName,
  const smtk::model::String& propValue)
{
  smtk::model::StringList tmp;
  tmp.push_back(propValue);
  this->setStringProperty(meshset, propName, tmp);
}

void Resource::setStringProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName,
  const smtk::model::StringList& propValue)
{
  if (meshset.size() > 0)
  {
    (*m_stringData)[meshset][propName] = propValue;
  }
}

smtk::model::StringList const& Resource::stringProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName) const
{
  if (meshset.size() > 0)
  {
    smtk::model::StringData& strings((*m_stringData)[meshset]);
    return strings[propName];
  }
  static smtk::model::StringList dummy;
  return dummy;
}

smtk::model::StringList& Resource::stringProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName)
{
  if (meshset.size() > 0)
  {
    smtk::model::StringData& strings((*m_stringData)[meshset]);
    return strings[propName];
  }
  static smtk::model::StringList dummy;
  return dummy;
}

bool Resource::hasStringProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName) const
{
  smtk::mesh::MeshStringData::const_iterator uit = m_stringData->find(meshset);
  if (uit == m_stringData->end())
  {
    return false;
  }
  smtk::model::StringData::const_iterator sit = uit->second.find(propName);
  // FIXME: Should we return true even when the array (*sit) is empty?
  return sit == uit->second.end() ? false : true;
}

bool Resource::removeStringProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName)
{
  smtk::mesh::MeshStringData::iterator uit = m_stringData->find(meshset);
  if (uit == m_stringData->end())
  {
    return false;
  }
  smtk::model::StringData::iterator sit = uit->second.find(propName);
  if (sit == uit->second.end())
  {
    return false;
  }
  uit->second.erase(sit);
  if (uit->second.empty())
    m_stringData->erase(uit);
  return true;
}

void Resource::setIntegerProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName, smtk::model::Integer propValue)
{
  smtk::model::IntegerList tmp;
  tmp.push_back(propValue);
  this->setIntegerProperty(meshset, propName, tmp);
}

void Resource::setIntegerProperty(const smtk::mesh::MeshSet& meshset, const std::string& propName,
  const smtk::model::IntegerList& propValue)
{
  if (meshset.size() > 0)
  {
    (*m_integerData)[meshset][propName] = propValue;
  }
}

smtk::model::IntegerList const& Resource::integerProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName) const
{
  if (meshset.size() > 0)
  {
    smtk::model::IntegerData& integers((*m_integerData)[meshset]);
    return integers[propName];
  }
  static smtk::model::IntegerList dummy;
  return dummy;
}

smtk::model::IntegerList& Resource::integerProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName)
{
  if (meshset.size() > 0)
  {
    smtk::model::IntegerData& integers((*m_integerData)[meshset]);
    return integers[propName];
  }
  static smtk::model::IntegerList dummy;
  return dummy;
}

bool Resource::hasIntegerProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName) const
{
  smtk::mesh::MeshIntegerData::const_iterator uit = m_integerData->find(meshset);
  if (uit == m_integerData->end())
  {
    return false;
  }
  smtk::model::IntegerData::const_iterator sit = uit->second.find(propName);
  // FIXME: Should we return true even when the array (*sit) is empty?
  return sit == uit->second.end() ? false : true;
}

bool Resource::removeIntegerProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName)
{
  smtk::mesh::MeshIntegerData::iterator uit = m_integerData->find(meshset);
  if (uit == m_integerData->end())
  {
    return false;
  }
  smtk::model::IntegerData::iterator sit = uit->second.find(propName);
  if (sit == uit->second.end())
  {
    return false;
  }
  uit->second.erase(sit);
  if (uit->second.empty())
    m_integerData->erase(uit);
  return true;
}
/*! \fn Resource::properties<T>()
 *  \brief Return a pointer to the properties of the resource.
 *
 * This templated version exists for use in functions where the
 * property type is a template parameter.
 */
template <>
SMTKCORE_EXPORT smtk::mesh::MeshStringData* Resource::properties<smtk::mesh::MeshStringData>()
{
  return &(*m_stringData);
}

template <>
SMTKCORE_EXPORT smtk::mesh::MeshFloatData* Resource::properties<smtk::mesh::MeshFloatData>()
{
  return &(*m_floatData);
}

template <>
SMTKCORE_EXPORT smtk::mesh::MeshIntegerData* Resource::properties<smtk::mesh::MeshIntegerData>()
{
  return &(*m_integerData);
}

/*! \fn Resource::meshProperties<T>(const smtk::mesh::MeshSet& meshset)
 *  \brief Return a pointer to the properties of an \a meshset in the resource.
 *
 * This templated version exists for use in functions where the
 * property type is a template parameter.
 */
template <>
SMTKCORE_EXPORT smtk::model::StringData* Resource::meshProperties<smtk::model::StringData>(
  const smtk::mesh::MeshSet& meshset)
{
  return &(*m_stringData)[meshset];
}

template <>
SMTKCORE_EXPORT smtk::model::FloatData* Resource::meshProperties<smtk::model::FloatData>(
  const smtk::mesh::MeshSet& meshset)
{
  return &(*m_floatData)[meshset];
}

template <>
SMTKCORE_EXPORT smtk::model::IntegerData* Resource::meshProperties<smtk::model::IntegerData>(
  const smtk::mesh::MeshSet& meshset)
{
  return &(*m_integerData)[meshset];
}

/*! \fn EntityRef::removeProperty<T>(const std::string& name)
 *  \brief Remove the property of type \a T with the given \a name, returning true on success.
 *
 * False is returned if the property did not exist for the given entity.
 *
 * This templated version exists for use in functions where the
 * property type is a template parameter.
 */
template <>
SMTKCORE_EXPORT bool Resource::removeProperty<smtk::model::StringData>(
  const smtk::mesh::MeshSet& meshset, const std::string& pname)
{
  return this->removeStringProperty(meshset, pname);
}

template <>
SMTKCORE_EXPORT bool Resource::removeProperty<smtk::model::FloatData>(
  const smtk::mesh::MeshSet& meshset, const std::string& pname)
{
  return this->removeFloatProperty(meshset, pname);
}

template <>
SMTKCORE_EXPORT bool Resource::removeProperty<smtk::model::IntegerData>(
  const smtk::mesh::MeshSet& meshset, const std::string& pname)
{
  return this->removeIntegerProperty(meshset, pname);
}
}
}
