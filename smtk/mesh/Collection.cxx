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
  m_internals( new InternalImpl() )
{

}

//----------------------------------------------------------------------------
Collection::Collection( smtk::mesh::ManagerPtr mngr ):
  m_entity( mngr->nextEntityId() ),
  m_name(),
  m_internals( new InternalImpl(mngr) )
{
}

//----------------------------------------------------------------------------
Collection::Collection( smtk::mesh::InterfacePtr interface,
                        smtk::mesh::ManagerPtr mngr):
  m_entity( mngr->nextEntityId() ),
  m_name(),
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
void Collection::removeManagerConnection()
{
  this->m_internals->resetManger();
}

//----------------------------------------------------------------------------
bool Collection::isValid() const
{
  //make sure we have a valid uuid, and that our internals are valid
  return this->m_entity.isNull() == false && this->m_internals->valid();
}

//----------------------------------------------------------------------------
const std::string& Collection::name() const
{
  return this->m_name;
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
smtk::mesh::TypeSet Collection::associatedTypes( ) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  return iface->computeTypes( this->m_internals->mesh_root_handle() );
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet Collection::cells( )
{
  smtk::mesh::MeshSet ms(this->shared_from_this(),
                         this->m_internals->mesh_root_handle());
  return ms.cells( );
}

//----------------------------------------------------------------------------
smtk::mesh::Points Collection::points( )
{
  return smtk::mesh::Points( );
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
  return iface->computeTypes(this->findAssociatedMeshes(eref).m_handle);
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
bool Collection::addAssociation( const smtk::model::EntityRef& eref ,
                                 const smtk::mesh::MeshSet& meshset )
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();
  // This causes the eref to become a meshset with the tag MODEL;
  // then all meshsets in m_range become child meshsets of eref:
  return iface->addAssociation( eref.entity(), meshset.m_range );
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
smtk::mesh::MeshSet Collection::createMesh( const smtk::mesh::CellSet& cells )
{
  const smtk::mesh::InterfacePtr& iface = this->m_internals->mesh_iface();

  smtk::mesh::HandleRange entities;
  if(cells.m_parent == this->shared_from_this())
    {
    smtk::mesh::Handle meshSetHandle;
    const bool meshCreated = iface->createMesh(cells.m_range, meshSetHandle);
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

    //find all non vertex cells that we use
    smtk::mesh::CellSet cellsUsedByDeletedMeshes = smtk::mesh::set_difference(
                                                     meshesToDelete.cells( ),
                                                     meshesToDelete.cells( smtk::mesh::Dims0 ));
    //now find the cells that used only by the mesh we are about to delete.
    //we can't delete vertex cells since they might be used as connectivity
    //for a different cell that we aren't deleting.
    cellsUsedByDeletedMeshes =
        smtk::mesh::set_difference(cellsUsedByDeletedMeshes,
                                    all_OtherMeshes.cells( ));

    //delete our mesh and cells that aren't used by any one else
    bool deletedMeshes = iface->deleteHandles(meshesToDelete.m_range);
    bool deletedCells = iface->deleteHandles(cellsUsedByDeletedMeshes.m_range);
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

}
}
