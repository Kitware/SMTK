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

namespace smtk {
namespace mesh {


//----------------------------------------------------------------------------
class Collection::InternalImpl
{
public:
  InternalImpl():
    WeakManager()
  {
  }

  InternalImpl( smtk::mesh::ManagerPtr mngr ):
    WeakManager(mngr)
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

  std::size_t size() const { return 0; }

private:
  smtk::weak_ptr<smtk::mesh::Manager> WeakManager;
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
  //first we need an uuid before we can be added to the manager
  if(mngr)
    {
    const bool valid = mngr->addCollection( *this );
    if (!valid)
      {
      this->m_entity = smtk::common::UUID::null();
      }
    }
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
std::size_t Collection::numberOfMeshes() const
{
  return this->m_internals->size();
}

//----------------------------------------------------------------------------
bool Collection::reparent(smtk::mesh::ManagerPtr newParent)
{
  //re-parent the collection onto a new manager
  smtk::mesh::ManagerPtr currentManager = this->m_internals->manager();
  if(currentManager)
    { //if we are assoicated with a valid manager remove the manager reference
      //to us before we re-parent our selves and this becomes impossible
      currentManager->removeCollection( *this );
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
  currentManager->addCollection( *this );

  return true;
}

//----------------------------------------------------------------------------
smtk::mesh::TypeSet Collection::associatedTypes( )
{
  return smtk::mesh::TypeSet();
}
//----------------------------------------------------------------------------
smtk::mesh::CellSet Collection::cells( )
{
  return smtk::mesh::CellSet();
}
//----------------------------------------------------------------------------
smtk::mesh::PointSet Collection::points( )
{
  return smtk::mesh::PointSet();
}
//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::meshes( )
{
  return smtk::mesh::MeshSet();
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet Collection::cells( smtk::mesh::CellType cellType )
{
  return smtk::mesh::CellSet();
}

// //----------------------------------------------------------------------------
smtk::mesh::CellSet Collection::cells( smtk::mesh::CellTypes cellTypes )
{
  return smtk::mesh::CellSet();
}

//----------------------------------------------------------------------------
smtk::mesh::PointSet Collection::pointsAssociatedTo( smtk::mesh::CellType cellType )
{
  return smtk::mesh::PointSet();
}
//----------------------------------------------------------------------------
smtk::mesh::PointSet Collection::pointsAssociatedTo( smtk::mesh::CellTypes cellTypes )
{
  return smtk::mesh::PointSet();
}

//----------------------------------------------------------------------------
smtk::mesh::TypeSet Collection::findAssociatedTypes( const smtk::model::EntityRef& eref )
{
  return smtk::mesh::TypeSet();
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet Collection::findAssociatedCells( const smtk::model::EntityRef& eref )
{
  return smtk::mesh::CellSet();
}
//----------------------------------------------------------------------------
smtk::mesh::PointSet Collection::findAssociatedPoints( const smtk::model::EntityRef& eref )
{
  return smtk::mesh::PointSet();
}
//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::findAssociatedMeshes( const smtk::model::EntityRef& eref )
{
  return smtk::mesh::MeshSet();
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet Collection::findAssociatedCells( const smtk::model::EntityRef& eref, smtk::mesh::CellType cellType )
{
  return smtk::mesh::CellSet();
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet Collection::findAssociatedMeshes( const smtk::model::EntityRef& eref, smtk::mesh::CellType cellType )
{
  return smtk::mesh::MeshSet();
}


}
}