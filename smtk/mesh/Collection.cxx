//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/mesh/moab/Interface.h"

namespace smtk {
namespace mesh {

//----------------------------------------------------------------------------
class Collection::InternalImpl
{
public:
  InternalImpl():
    WeakManager(),
    Interface( smtk::mesh::moab::make_interface()  )
  {
  }

  InternalImpl( smtk::mesh::ManagerPtr mngr ):
    WeakManager(mngr),
    Interface( smtk::mesh::moab::make_interface() )
  {
  }

  InternalImpl( smtk::mesh::ManagerPtr mngr,
                smtk::mesh::InterfacePtr interface ):
    WeakManager(mngr),
    Interface( interface )
  {
  }

  void resetManger()
    { this->WeakManager.reset(); }

  bool valid() const
    { return !this->WeakManager.expired(); }

  smtk::shared_ptr<smtk::mesh::Manager> manager()
  { return this->WeakManager.lock(); }

  bool reparent( smtk::mesh::ManagerPtr newMngr)
    {
    this->resetManger();
    this->WeakManager = smtk::weak_ptr<smtk::mesh::Manager>(newMngr);
    return true;
    }

  const smtk::mesh::InterfacePtr& mesh_iface() const
    { return this->Interface; }

  smtk::mesh::Handle mesh_root_handle() const
    { return this->Interface->getRoot(); }

private:
  smtk::weak_ptr<smtk::mesh::Manager> WeakManager;
  smtk::mesh::InterfacePtr Interface;
};

//----------------------------------------------------------------------------
Collection::Collection():
  m_entity( smtk::common::UUID::null() ),
  m_name(),
  m_readLocation(),
  m_writeLocation(),
  m_floatData(new MeshFloatData),
  m_stringData(new MeshStringData),
  m_integerData(new MeshIntegerData),
  m_internals( new InternalImpl() )
{

}

//----------------------------------------------------------------------------
Collection::Collection(const smtk::common::UUID& collectionID,
                       smtk::mesh::ManagerPtr mngr ):
  m_entity( collectionID ),
  m_name(),
  m_readLocation(),
  m_writeLocation(),
  m_floatData(new MeshFloatData),
  m_stringData(new MeshStringData),
  m_integerData(new MeshIntegerData),
  m_internals( new InternalImpl(mngr) )
{
}

//----------------------------------------------------------------------------
Collection::Collection( const smtk::common::UUID& collectionID,
                        smtk::mesh::InterfacePtr interface,
                        smtk::mesh::ManagerPtr mngr):
  m_entity( collectionID ),
  m_name(),
  m_readLocation(),
  m_writeLocation(),
  m_floatData(new MeshFloatData),
  m_stringData(new MeshStringData),
  m_integerData(new MeshIntegerData),
  m_internals( new InternalImpl(mngr, interface) )
{

}

//----------------------------------------------------------------------------
Collection::~Collection()
{
  if(this->m_internals)
    {
    delete this->m_internals;
    }
}

//----------------------------------------------------------------------------
const smtk::mesh::InterfacePtr& Collection::interface() const
{
  return this->m_internals->mesh_iface();
}

//----------------------------------------------------------------------------
void Collection::swapInterfaces(smtk::mesh::CollectionPtr& other)
{
  smtk::mesh::Collection::InternalImpl* temp = other->m_internals;
  other->m_internals = this->m_internals;
  this->m_internals = temp;
}

//----------------------------------------------------------------------------
void Collection::removeManagerConnection()
{
  this->m_internals->resetManger();
}

//----------------------------------------------------------------------------
bool Collection::isValid() const
{
  //make sure we have a valid uuid, and that our internals are valid
  return (this->m_entity.isNull() != true) &&
         (this->m_internals->valid());
}

//----------------------------------------------------------------------------
bool Collection::isModified() const
{
  //make sure we have a valid uuid, and that our internals are valid
  return this->interface()->isModified();
}

//----------------------------------------------------------------------------
const std::string& Collection::name() const
{
  return this->m_name;
}

//----------------------------------------------------------------------------
void Collection::name(const std::string& n)
{
  this->m_name = n;
}

//----------------------------------------------------------------------------
bool Collection::assignUniqueNameIfNotAlready()
{
  smtk::mesh::ManagerPtr currentManager = this->m_internals->manager();
  if(currentManager)
    { //if we are associated with a valid manager
      return currentManager->assignUniqueName( this->shared_from_this() );
    }
  return false;
}

//----------------------------------------------------------------------------
const smtk::common::FileLocation& Collection::readLocation() const
{
  return this->m_readLocation;
}

//----------------------------------------------------------------------------
void Collection::readLocation(const smtk::common::FileLocation& n)
{
  this->m_readLocation = n;
  //if the write location hasn't been set, update it to be the read location
  if(this->m_writeLocation.empty())
    {
    this->m_writeLocation = n;
    }
}

//----------------------------------------------------------------------------
const smtk::common::FileLocation& Collection::writeLocation() const
{
  return this->m_writeLocation;
}

//----------------------------------------------------------------------------
void Collection::writeLocation(const smtk::common::FileLocation& n)
{
  this->m_writeLocation = n;
}

//----------------------------------------------------------------------------
void Collection::clearReadWriteLocations()
{
  this->m_readLocation.clear();
  this->m_writeLocation.clear();
}

//----------------------------------------------------------------------------
std::string Collection::interfaceName() const
{
  return this->interface()->name();
}

//----------------------------------------------------------------------------
const smtk::common::UUID Collection::entity() const
{
  return this->m_entity;
}

//----------------------------------------------------------------------------
bool Collection::reparent(smtk::mesh::ManagerPtr newParent)
{
  //re-parent the collection onto a new manager
  smtk::mesh::ManagerPtr currentManager = this->m_internals->manager();
  if(currentManager)
    { //if we are assoicated with a valid manager remove the manager reference
      //to us before we re-parent our selves and this becomes impossible
      currentManager->removeCollection( this->shared_from_this() );
    }

  const bool reparenting = this->m_internals->reparent(newParent);
  (void)reparenting;
  currentManager = this->m_internals->manager();

  //we need to get a uuid if we don't have one already
  if(!currentManager)
    {
    return false;
    }

  //if we currently don't have a uuid get one
  if(this->m_entity.isNull())
    {
    this->m_entity = currentManager->nextEntityId();
    }

  //add us to the new manager
  currentManager->addCollection( this->shared_from_this() );

  return true;
}

//----------------------------------------------------------------------------
std::size_t Collection::numberOfMeshes() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  return iface->numMeshes( this->m_internals->mesh_root_handle() );
}

//----------------------------------------------------------------------------
smtk::mesh::TypeSet Collection::types() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  smtk::mesh::moab::Handle handle = this->m_internals->mesh_root_handle();
  return iface->computeTypes( iface->getMeshsets(handle) );
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet Collection::cells( )
{
  smtk::mesh::MeshSet ms(this->shared_from_this(),
                         this->m_internals->mesh_root_handle());
  return ms.cells( );
}

//----------------------------------------------------------------------------
smtk::mesh::PointSet Collection::points( )
{
  smtk::mesh::MeshSet ms(this->shared_from_this(),
                         this->m_internals->mesh_root_handle());
  return ms.points( );
}

//----------------------------------------------------------------------------
smtk::mesh::PointConnectivity Collection::pointConnectivity( )
{
  smtk::mesh::MeshSet ms(this->shared_from_this(),
                         this->m_internals->mesh_root_handle());
  return ms.pointConnectivity();
}


//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::meshes( )
{
  return smtk::mesh::MeshSet( this->shared_from_this(),
                               this->m_internals->mesh_root_handle() );
}

//----------------------------------------------------------------------------
std::vector< std::string > Collection::meshNames( )
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  smtk::mesh::moab::Handle handle = this->m_internals->mesh_root_handle();

  smtk::mesh::HandleRange entities = iface->getMeshsets(handle);
  return iface->computeNames(entities);
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::meshes( smtk::mesh::DimensionType dim )
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  smtk::mesh::moab::Handle handle = this->m_internals->mesh_root_handle();
  const int dim_value = static_cast<int>(dim);

  smtk::mesh::HandleRange entities = iface->getMeshsets( handle, dim_value);
  return smtk::mesh::MeshSet( this->shared_from_this(),
                              this->m_internals->mesh_root_handle(),
                              entities );
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::meshes( const std::string& name )
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  smtk::mesh::moab::Handle handle = this->m_internals->mesh_root_handle();

  smtk::mesh::HandleRange entities = iface->getMeshsets( handle, name);
  return smtk::mesh::MeshSet( this->shared_from_this(),
                              this->m_internals->mesh_root_handle(),
                              entities );
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::meshes(const smtk::mesh::Domain& d )
{
  return this->domainMeshes(d);
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::meshes(const smtk::mesh::Dirichlet& d )
{
  return this->dirichletMeshes(d);
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::meshes(const smtk::mesh::Neumann& n)
{
  return this->neumannMeshes(n);
}


//----------------------------------------------------------------------------
smtk::mesh::CellSet Collection::cells( smtk::mesh::CellType cellType )
{
  smtk::mesh::MeshSet ms(this->shared_from_this(),
                         this->m_internals->mesh_root_handle());
  return ms.cells( cellType );
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet Collection::cells( smtk::mesh::CellTypes cellTypes )
{
  smtk::mesh::MeshSet ms(this->shared_from_this(),
                         this->m_internals->mesh_root_handle());
  return ms.cells( cellTypes );
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet Collection::cells( smtk::mesh::DimensionType dim )
{
  smtk::mesh::MeshSet ms(this->shared_from_this(),
                         this->m_internals->mesh_root_handle());
  return ms.cells( dim );
}

//----------------------------------------------------------------------------
smtk::mesh::TypeSet Collection::findAssociatedTypes( const smtk::model::EntityRef& eref )
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  return this->findAssociatedMeshes(eref).types();
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::findAssociatedMeshes(const smtk::model::EntityRef& eref)
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  smtk::mesh::moab::Handle handle = this->m_internals->mesh_root_handle();

  return smtk::mesh::MeshSet(
    this->shared_from_this(), handle,
    iface->findAssociations(handle, eref.entity()));
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::findAssociatedMeshes( const smtk::model::EntityRef& eref ,
                                                      smtk::mesh::DimensionType dim )
{
  smtk::mesh::MeshSet unfiltered = this->findAssociatedMeshes(eref);
  return unfiltered.subset(dim);
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet Collection::findAssociatedCells( const smtk::model::EntityRef& eref  )
{
  smtk::mesh::MeshSet ms = this->findAssociatedMeshes(eref);
  return ms.cells();
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet Collection::findAssociatedCells( const smtk::model::EntityRef& eref, smtk::mesh::CellType cellType )
{
  smtk::mesh::MeshSet ms = this->findAssociatedMeshes(eref);
  return ms.cells(cellType);
}


//----------------------------------------------------------------------------
smtk::mesh::CellSet Collection::findAssociatedCells( const smtk::model::EntityRef& eref ,
                                                      smtk::mesh::DimensionType dim )
{
  smtk::mesh::MeshSet ms = this->findAssociatedMeshes(eref, dim);
  return ms.cells();
}


//----------------------------------------------------------------------------
bool Collection::setAssociation( const smtk::model::EntityRef& eref ,
                                 const smtk::mesh::MeshSet& meshset )
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  // This causes the eref to become a meshset with the tag MODEL;
  // then all meshsets in m_range become child meshsets of eref:
  return iface->setAssociation( eref.entity(), meshset.m_range );
}

//----------------------------------------------------------------------------
bool Collection::hasAssociations( ) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();

  smtk::mesh::moab::Handle handle = this->m_internals->mesh_root_handle();
  smtk::mesh::HandleRange entities = iface->getMeshsets(handle);

  smtk::common::UUIDArray associations = iface->computeModelEntities(entities);
  return !associations.empty();
}

//----------------------------------------------------------------------------
bool Collection::associateToModel(const smtk::common::UUID& uuid)
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();

  //before we associate this model make sure we are not already associated
  //to this model UUID. This is needed so that we are serializing/de-serializing
  //meshes we don't set the modify bit to true, when we re-associate to the model
  //and already had that info
  if(iface->rootAssociation() != uuid)
    {
    return iface->setRootAssociation( uuid );
    }
  return true;
}

//----------------------------------------------------------------------------
bool Collection::isAssociatedToModel() const
{
  return this->associatedModel() != smtk::common::UUID::null();
}

//----------------------------------------------------------------------------
smtk::common::UUID Collection::associatedModel() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  return iface->rootAssociation( );
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::createMesh( const smtk::mesh::CellSet& cells )
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();

  smtk::mesh::HandleRange entities;
  if(cells.m_parent == this->shared_from_this())
    {
    smtk::mesh::Handle meshSetHandle;
    const bool meshCreated = iface->createMesh(cells.range(), meshSetHandle);
    if(meshCreated)
      {
      entities.insert(meshSetHandle);
      }
    }
  return smtk::mesh::MeshSet( this->shared_from_this(),
                              this->m_internals->mesh_root_handle(),
                              entities );
}

//----------------------------------------------------------------------------
bool Collection::removeMeshes(smtk::mesh::MeshSet& meshesToDelete )
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  if(meshesToDelete.m_parent == this->shared_from_this())
    {
    //When deleting a MeshSet we need to find all cells that
    //are not used by any other mesh.

    //find all other meshes
    smtk::mesh::MeshSet all_OtherMeshes =
                      smtk::mesh::set_difference(this->meshes(),
                                                 meshesToDelete);

    //now find the cells that are only used by the mesh we are about to delete.
    smtk::mesh::CellSet cellsToDelete = smtk::mesh::set_difference(meshesToDelete.cells(),
                                                                   all_OtherMeshes.cells( ));
    //delete our mesh and cells that aren't used by any one else
    const bool deletedMeshes = iface->deleteHandles(meshesToDelete.m_range);
    const bool deletedCells = iface->deleteHandles(cellsToDelete.range());
    return deletedMeshes && deletedCells;
    }
  return false;
}

//----------------------------------------------------------------------------
std::vector< smtk::mesh::Domain > Collection::domains()
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  smtk::mesh::moab::Handle handle = this->m_internals->mesh_root_handle();

  smtk::mesh::HandleRange entities = iface->getMeshsets( handle );
  return iface->computeDomainValues( entities );
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::domainMeshes( const smtk::mesh::Domain& d )
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  smtk::mesh::moab::Handle handle = this->m_internals->mesh_root_handle();

  smtk::mesh::HandleRange entities = iface->getMeshsets( handle, d);
  return smtk::mesh::MeshSet( this->shared_from_this(),
                              this->m_internals->mesh_root_handle(),
                              entities );
}

//----------------------------------------------------------------------------
bool Collection::setDomainOnMeshes(const smtk::mesh::MeshSet& meshes,
                                   const smtk::mesh::Domain &d)
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  if(meshes.m_parent == this->shared_from_this())
    {
    return iface->setDomain(meshes.m_range,d);
    }
  return false;
}

//----------------------------------------------------------------------------
std::vector< smtk::mesh::Dirichlet > Collection::dirichlets()
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  smtk::mesh::moab::Handle handle = this->m_internals->mesh_root_handle();

  smtk::mesh::HandleRange entities = iface->getMeshsets( handle );
  return iface->computeDirichletValues( entities );
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::dirichletMeshes( const smtk::mesh::Dirichlet& d )
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  smtk::mesh::moab::Handle handle = this->m_internals->mesh_root_handle();

  smtk::mesh::HandleRange entities = iface->getMeshsets( handle, d);
  return smtk::mesh::MeshSet( this->shared_from_this(),
                              this->m_internals->mesh_root_handle(),
                              entities);
}

//----------------------------------------------------------------------------
bool Collection::setDirichletOnMeshes(const smtk::mesh::MeshSet& meshes,
                                      const smtk::mesh::Dirichlet &d)
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  if(meshes.m_parent == this->shared_from_this())
    {
    return iface->setDirichlet(meshes.m_range,d);
    }
  return false;
}

//----------------------------------------------------------------------------
std::vector< smtk::mesh::Neumann > Collection::neumanns()
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  smtk::mesh::moab::Handle handle = this->m_internals->mesh_root_handle();

  smtk::mesh::HandleRange entities = iface->getMeshsets( handle );
  return iface->computeNeumannValues( entities );
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::neumannMeshes( const smtk::mesh::Neumann& n )
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  smtk::mesh::moab::Handle handle = this->m_internals->mesh_root_handle();

  smtk::mesh::HandleRange entities = iface->getMeshsets( handle, n);
  return smtk::mesh::MeshSet( this->shared_from_this(),
                              this->m_internals->mesh_root_handle(),
                              entities);
}

//----------------------------------------------------------------------------
bool Collection::setNeumannOnMeshes(const smtk::mesh::MeshSet& meshes,
                                    const smtk::mesh::Neumann &n)
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  if(meshes.m_parent == this->shared_from_this())
    {
    return iface->setNeumann(meshes.m_range,n);
    }
  return false;
}

//----------------------------------------------------------------------------
/** @name Model property accessors.
  *
  */
///@{
void Collection::setFloatProperty(
  const smtk::mesh::MeshSet& meshset,
  const std::string& propName,
  smtk::model::Float propValue)
{
  smtk::model::FloatList tmp;
  tmp.push_back(propValue);
  this->setFloatProperty(meshset, propName, tmp);
}

void Collection::setFloatProperty(
  const smtk::mesh::MeshSet& meshset,
  const std::string& propName,
  const smtk::model::FloatList& propValue)
{
  if (meshset.size() > 0)
    {
    (*this->m_floatData)[meshset][propName] = propValue;
    }
}

smtk::model::FloatList const& Collection::floatProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName) const
{
  if (meshset.size() > 0)
    {
    smtk::model::FloatData& floats((*this->m_floatData)[meshset]);
    return floats[propName];
    }
  static smtk::model::FloatList dummy;
  return dummy;
}

smtk::model::FloatList& Collection::floatProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName)
{
  if (meshset.size() > 0)
    {
    smtk::model::FloatData& floats((*this->m_floatData)[meshset]);
    return floats[propName];
    }
  static smtk::model::FloatList dummy;
  return dummy;
}

bool Collection::hasFloatProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName) const
{
  smtk::mesh::MeshFloatData::const_iterator uit = this->m_floatData->find(meshset);
  if (uit == this->m_floatData->end())
    {
    return false;
    }
  smtk::model::FloatData::const_iterator sit = uit->second.find(propName);
  // FIXME: Should we return true even when the array (*sit) is empty?
  return sit == uit->second.end() ? false : true;
}

bool Collection::removeFloatProperty(
  const smtk::mesh::MeshSet& meshset,
  const std::string& propName)
{
  smtk::mesh::MeshFloatData::iterator uit = this->m_floatData->find(meshset);
  if (uit == this->m_floatData->end())
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
    this->m_floatData->erase(uit);
  return true;
}

void Collection::setStringProperty(
  const smtk::mesh::MeshSet& meshset,
  const std::string& propName,
  const smtk::model::String& propValue)
{
  smtk::model::StringList tmp;
  tmp.push_back(propValue);
  this->setStringProperty(meshset, propName, tmp);
}

void Collection::setStringProperty(
  const smtk::mesh::MeshSet& meshset,
  const std::string& propName,
  const smtk::model::StringList& propValue)
{
  if (meshset.size() > 0)
    {
    (*this->m_stringData)[meshset][propName] = propValue;
    }
}

smtk::model::StringList const& Collection::stringProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName) const
{
  if (meshset.size() > 0)
    {
    smtk::model::StringData& strings((*this->m_stringData)[meshset]);
    return strings[propName];
    }
  static smtk::model::StringList dummy;
  return dummy;
}

smtk::model::StringList& Collection::stringProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName)
{
  if (meshset.size() > 0)
    {
    smtk::model::StringData& strings((*this->m_stringData)[meshset]);
    return strings[propName];
    }
  static smtk::model::StringList dummy;
  return dummy;
}

bool Collection::hasStringProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName) const
{
  smtk::mesh::MeshStringData::const_iterator uit = this->m_stringData->find(meshset);
  if (uit == this->m_stringData->end())
    {
    return false;
    }
  smtk::model::StringData::const_iterator sit = uit->second.find(propName);
  // FIXME: Should we return true even when the array (*sit) is empty?
  return sit == uit->second.end() ? false : true;
}

bool Collection::removeStringProperty(
  const smtk::mesh::MeshSet& meshset,
  const std::string& propName)
{
  smtk::mesh::MeshStringData::iterator uit = this->m_stringData->find(meshset);
  if (uit == this->m_stringData->end())
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
    this->m_stringData->erase(uit);
  return true;
}

void Collection::setIntegerProperty(
  const smtk::mesh::MeshSet& meshset,
  const std::string& propName,
  smtk::model::Integer propValue)
{
  smtk::model::IntegerList tmp;
  tmp.push_back(propValue);
  this->setIntegerProperty(meshset, propName, tmp);
}

void Collection::setIntegerProperty(
  const smtk::mesh::MeshSet& meshset,
  const std::string& propName,
  const smtk::model::IntegerList& propValue)
{
  if (meshset.size() > 0)
    {
    (*this->m_integerData)[meshset][propName] = propValue;
    }
}

smtk::model::IntegerList const& Collection::integerProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName) const
{
  if (meshset.size() > 0)
    {
    smtk::model::IntegerData& integers((*this->m_integerData)[meshset]);
    return integers[propName];
    }
  static smtk::model::IntegerList dummy;
  return dummy;
}

smtk::model::IntegerList& Collection::integerProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName)
{
  if (meshset.size() > 0)
    {
    smtk::model::IntegerData& integers((*this->m_integerData)[meshset]);
    return integers[propName];
    }
  static smtk::model::IntegerList dummy;
  return dummy;
}

bool Collection::hasIntegerProperty(
  const smtk::mesh::MeshSet& meshset, const std::string& propName) const
{
  smtk::mesh::MeshIntegerData::const_iterator uit = this->m_integerData->find(meshset);
  if (uit == this->m_integerData->end())
    {
    return false;
    }
  smtk::model::IntegerData::const_iterator sit = uit->second.find(propName);
  // FIXME: Should we return true even when the array (*sit) is empty?
  return sit == uit->second.end() ? false : true;
}

bool Collection::removeIntegerProperty(
  const smtk::mesh::MeshSet& meshset,
  const std::string& propName)
{
  smtk::mesh::MeshIntegerData::iterator uit = this->m_integerData->find(meshset);
  if (uit == this->m_integerData->end())
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
    this->m_integerData->erase(uit);
  return true;
}
/*! \fn Collection::properties<T>()
 *  \brief Return a pointer to the properties of the collection.
 *
 * This templated version exists for use in functions where the
 * property type is a template parameter.
 */
template<>
SMTKCORE_EXPORT
smtk::mesh::MeshStringData* Collection::properties<smtk::mesh::MeshStringData>()
{ return &(*this->m_stringData); }

template<>
SMTKCORE_EXPORT
smtk::mesh::MeshFloatData* Collection::properties<smtk::mesh::MeshFloatData>()
{ return &(*this->m_floatData); }

template<>
SMTKCORE_EXPORT
smtk::mesh::MeshIntegerData* Collection::properties<smtk::mesh::MeshIntegerData>()
{ return &(*this->m_integerData); }

/*! \fn Collection::meshProperties<T>(const smtk::mesh::MeshSet& meshset)
 *  \brief Return a pointer to the properties of an \a meshset in the collection.
 *
 * This templated version exists for use in functions where the
 * property type is a template parameter.
 */
template<>
SMTKCORE_EXPORT
smtk::model::StringData* Collection::meshProperties<smtk::model::StringData>(
  const smtk::mesh::MeshSet& meshset)
{ return &(*this->m_stringData)[meshset]; }

template<>
SMTKCORE_EXPORT
smtk::model::FloatData* Collection::meshProperties<smtk::model::FloatData>(
  const smtk::mesh::MeshSet& meshset)
{ return &(*this->m_floatData)[meshset]; }

template<>
SMTKCORE_EXPORT
smtk::model::IntegerData* Collection::meshProperties<smtk::model::IntegerData>(
  const smtk::mesh::MeshSet& meshset)
{ return &(*this->m_integerData)[meshset]; }

/*! \fn EntityRef::removeProperty<T>(const std::string& name)
 *  \brief Remove the property of type \a T with the given \a name, returning true on success.
 *
 * False is returned if the property did not exist for the given entity.
 *
 * This templated version exists for use in functions where the
 * property type is a template parameter.
 */
template<>
SMTKCORE_EXPORT
bool Collection::removeProperty<smtk::model::StringData>(
  const smtk::mesh::MeshSet& meshset, const std::string& pname)
{ return this->removeStringProperty(meshset, pname); }

template<>
SMTKCORE_EXPORT
bool Collection::removeProperty<smtk::model::FloatData>(
  const smtk::mesh::MeshSet& meshset, const std::string& pname)
{ return this->removeFloatProperty(meshset, pname); }

template<>
SMTKCORE_EXPORT
bool Collection::removeProperty<smtk::model::IntegerData>(
  const smtk::mesh::MeshSet& meshset, const std::string& pname)
{ return this->removeIntegerProperty(meshset, pname); }

//----------------------------------------------------------------------------

}
}
