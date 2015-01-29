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
               smtk::mesh::moab::InterfacePtr interface ):
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

  const smtk::mesh::moab::InterfacePtr& mesh_iface() const
    { return this->Interface; }

  smtk::mesh::Handle mesh_root_handle() const
    { return this->Interface->get_root_set(); }



private:
  smtk::weak_ptr<smtk::mesh::Manager> WeakManager;
  smtk::mesh::moab::InterfacePtr Interface;
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
Collection::Collection( smtk::mesh::moab::InterfacePtr interface,
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
const smtk::mesh::moab::InterfacePtr& Collection::extractInterface() const
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
  return smtk::mesh::moab::numMeshes(this->m_internals->mesh_root_handle(),
                                     this->m_internals->mesh_iface() );
}

//----------------------------------------------------------------------------
smtk::mesh::TypeSet Collection::associatedTypes( ) const
{
  return smtk::mesh::moab::compute_types(this->m_internals->mesh_root_handle(),
                                         this->m_internals->mesh_iface() );
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
  const smtk::mesh::moab::InterfacePtr& iface = this->m_internals->mesh_iface();
  smtk::mesh::moab::Handle handle = this->m_internals->mesh_root_handle();

  ::moab::Range entities = smtk::mesh::moab::get_meshsets(handle, iface);
  return smtk::mesh::moab::compute_names(entities, iface);
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::meshes( smtk::mesh::DimensionType dim )
{
  const smtk::mesh::moab::InterfacePtr& iface = this->m_internals->mesh_iface();
  smtk::mesh::moab::Handle handle = this->m_internals->mesh_root_handle();
  const int dim_value = static_cast<int>(dim);

  smtk::mesh::HandleRange entities = smtk::mesh::moab::get_meshsets( handle,
                                                                     dim_value,
                                                                     iface );
  return smtk::mesh::MeshSet( this->shared_from_this(),
                              this->m_internals->mesh_root_handle(),
                              entities );
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::meshes( const std::string& name )
{
  const smtk::mesh::moab::InterfacePtr& iface = this->m_internals->mesh_iface();
  smtk::mesh::moab::Handle handle = this->m_internals->mesh_root_handle();

  smtk::mesh::HandleRange entities = smtk::mesh::moab::get_meshsets( handle,
                                                                     name,
                                                                     iface );
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
  return smtk::mesh::TypeSet();
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::findAssociatedMeshes( const smtk::model::EntityRef& eref  )
{
  return smtk::mesh::MeshSet( this->shared_from_this(),
                              this->m_internals->mesh_root_handle(),
                              smtk::mesh::HandleRange() );
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::findAssociatedMeshes( const smtk::model::EntityRef& eref ,
                                                      smtk::mesh::CellType cellType )
{
  return smtk::mesh::MeshSet( this->shared_from_this(),
                              this->m_internals->mesh_root_handle(),
                              smtk::mesh::HandleRange() );
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::findAssociatedMeshes( const smtk::model::EntityRef& eref ,
                                                      smtk::mesh::DimensionType dim )
{
  return smtk::mesh::MeshSet( this->shared_from_this(),
                              this->m_internals->mesh_root_handle(),
                              smtk::mesh::HandleRange() );
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet Collection::findAssociatedCells( const smtk::model::EntityRef& eref  )
{
  smtk::mesh::MeshSet ms(this->shared_from_this(),
                         this->m_internals->mesh_root_handle());
  return ms.cells( );
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet Collection::findAssociatedCells( const smtk::model::EntityRef& eref, smtk::mesh::CellType cellType )
{
  smtk::mesh::MeshSet ms(this->shared_from_this(),
                         this->m_internals->mesh_root_handle());
  return ms.cells( );
}


//----------------------------------------------------------------------------
smtk::mesh::CellSet Collection::findAssociatedCells( const smtk::model::EntityRef& eref ,
                                                      smtk::mesh::DimensionType dim )
{
  smtk::mesh::MeshSet ms(this->shared_from_this(),
                         this->m_internals->mesh_root_handle());
  return ms.cells( );
}


}
}
