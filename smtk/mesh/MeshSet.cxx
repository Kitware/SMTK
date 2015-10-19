//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/Collection.h"

#include "smtk/mesh/Interface.h"

namespace smtk {
namespace mesh {

//----------------------------------------------------------------------------
MeshSet::MeshSet():
  m_parent(),
  m_handle(),
  m_range()
{
  //Trying to make Shitbroken happy
}

//----------------------------------------------------------------------------
MeshSet::MeshSet(const smtk::mesh::CollectionPtr& parent,
                 smtk::mesh::Handle handle)
{
  this->m_parent = parent;
  this->m_handle = handle;

  const smtk::mesh::InterfacePtr& iface = parent->interface();
  //range of moab entity sets
  this->m_range = iface->getMeshsets( handle );
}

//----------------------------------------------------------------------------
MeshSet::MeshSet(const smtk::mesh::CollectionPtr& parent,
                 smtk::mesh::Handle handle,
                 const smtk::mesh::HandleRange& range):
  m_parent(parent),
  m_handle(handle),
  m_range(range) //range of moab entity sets
{

}

//----------------------------------------------------------------------------
MeshSet::MeshSet(const smtk::mesh::MeshSet& other):
  m_parent(other.m_parent),
  m_handle(other.m_handle),
  m_range(other.m_range)
{

}

//----------------------------------------------------------------------------
MeshSet::~MeshSet()
{

}

//----------------------------------------------------------------------------
MeshSet& MeshSet::operator=(const MeshSet& other)
{
  this->m_parent = other.m_parent;
  this->m_handle = other.m_handle;
  this->m_range = other.m_range;
  return *this;
}

//----------------------------------------------------------------------------
bool MeshSet::operator==(const MeshSet& other) const
{
  return this->m_parent == other.m_parent &&
         this->m_handle == other.m_handle &&
         //empty is a fast way to check for easy mismatching ranges
         this->m_range.empty() == other.m_range.empty()  &&
         this->m_range == other.m_range;
}

//----------------------------------------------------------------------------
bool MeshSet::operator!=(const MeshSet& other) const
{
  return !(*this == other);
}

//----------------------------------------------------------------------------
bool MeshSet::append( const MeshSet& other)
{
  const bool can_append = this->m_parent == other.m_parent &&
                          this->m_handle == other.m_handle;
  if(can_append)
    {
    this->m_range.insert(other.m_range.begin(), other.m_range.end());
    }
  return can_append;
}

//----------------------------------------------------------------------------
bool MeshSet::is_empty( ) const
{
  return this->m_range.empty();
}

//----------------------------------------------------------------------------
std::size_t MeshSet::size( ) const
{
  return this->m_range.size();
}

//----------------------------------------------------------------------------
std::vector< smtk::mesh::Domain > MeshSet::domains( ) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->computeDomainValues( this->m_range );
}

//----------------------------------------------------------------------------
std::vector< smtk::mesh::Dirichlet > MeshSet::dirichlets( ) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->computeDirichletValues( this->m_range );
}

//----------------------------------------------------------------------------
std::vector< smtk::mesh::Neumann > MeshSet::neumanns( ) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->computeNeumannValues( this->m_range );
}

//----------------------------------------------------------------------------
bool MeshSet::setDomain(const smtk::mesh::Domain& d)
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->setDomain( this->m_range, d);
}

//----------------------------------------------------------------------------
bool MeshSet::setDirichlet(const smtk::mesh::Dirichlet& d)
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->setDirichlet( this->m_range, d );
}

//----------------------------------------------------------------------------
bool MeshSet::setNeumann(const smtk::mesh::Neumann& n)
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->setNeumann( this->m_range, n );
}

/**\brief Return an array of model entity UUIDs associated with meshset members.
  *
  */
smtk::common::UUIDArray MeshSet::modelEntityIds() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->computeModelEntities(this->m_range);
}

/**\brief Return the model entities associated with meshset members.
  *
  * warning Note that the parent collection of this meshset must have
  *         its model manager set to a valid value or the result will
  *         be an array of invalid entries.
  */
smtk::model::EntityRefArray MeshSet::modelEntities() const
{
  smtk::common::UUIDArray uids = this->modelEntityIds();
  smtk::model::EntityRefArray result;
  smtk::model::ManagerPtr mgr = this->m_parent->modelManager();
  for (smtk::common::UUIDArray::const_iterator it = uids.begin(); it != uids.end(); ++it)
    result.push_back(smtk::model::EntityRef(mgr, *it));
  return result;
}

/**\brief Set the model entity for each meshset member to \a ent.
  *
  */
bool MeshSet::setModelEntities(const smtk::model::EntityRef& ent)
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->setModelEntity(this->m_range, ent.entity());
}

//----------------------------------------------------------------------------
smtk::mesh::TypeSet MeshSet::types() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->computeTypes( this->m_range );
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet MeshSet::cells( ) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells( this->m_range );
  return smtk::mesh::CellSet(this->m_parent, range);
}

//----------------------------------------------------------------------------
smtk::mesh::PointSet MeshSet::points( ) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange cells = iface->getCells( this->m_range );
  smtk::mesh::HandleRange range = iface->getPoints( cells );
  return smtk::mesh::PointSet(this->m_parent, range);
}

//----------------------------------------------------------------------------
smtk::mesh::PointConnectivity MeshSet::pointConnectivity( ) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells( this->m_range );
  return smtk::mesh::PointConnectivity(this->m_parent, range);
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet MeshSet::cells( smtk::mesh::CellType cellType ) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells( this->m_range, cellType );
  return smtk::mesh::CellSet(this->m_parent, range);
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet MeshSet::cells( smtk::mesh::CellTypes cellTypes ) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells( this->m_range, cellTypes );
  return smtk::mesh::CellSet(this->m_parent, range);
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet MeshSet::cells( smtk::mesh::DimensionType dim ) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells( this->m_range, dim );
  return smtk::mesh::CellSet(this->m_parent, range);
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet MeshSet::subset( smtk::mesh::DimensionType dim ) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange dimMeshes = iface->getMeshsets(this->m_handle, dim);
  //intersect our mesh id with those of a given dimension to find the subset
  return smtk::mesh::MeshSet(this->m_parent,
                             this->m_handle,
                             iface->rangeIntersect(dimMeshes,this->m_range));
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet MeshSet::subset( const smtk::mesh::Domain& d ) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange dMeshes = iface->getMeshsets(this->m_handle, d);
  //intersect our mesh id with those of a given dimension to find the subset
  return smtk::mesh::MeshSet(this->m_parent,
                             this->m_handle,
                             iface->rangeIntersect(dMeshes,this->m_range));
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet MeshSet::subset( const smtk::mesh::Dirichlet& d ) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange dMeshes = iface->getMeshsets(this->m_handle, d);
  //intersect our mesh id with those of a given dimension to find the subset
  return smtk::mesh::MeshSet(this->m_parent,
                             this->m_handle,
                             iface->rangeIntersect(dMeshes,this->m_range));
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet MeshSet::subset( const smtk::mesh::Neumann& n ) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange nMeshes = iface->getMeshsets(this->m_handle, n);
  //intersect our mesh id with those of a given dimension to find the subset
  return smtk::mesh::MeshSet(this->m_parent,
                             this->m_handle,
                             iface->rangeIntersect(nMeshes,this->m_range));
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet MeshSet::extractShell() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();

  smtk::mesh::HandleRange entities;
  smtk::mesh::HandleRange cells;
  const bool shellExtracted = iface->computeShell( this->m_range, cells );
  if(shellExtracted)
    {
    smtk::mesh::Handle meshSetHandle;
    //create a mesh for these cells since they don't have a meshset currently
    const bool meshCreated = iface->createMesh(cells, meshSetHandle);
    if(meshCreated)
      {
      entities.insert(meshSetHandle);
      }
    }
  return smtk::mesh::MeshSet( this->m_parent,
                              this->m_handle,
                              entities );
}

//----------------------------------------------------------------------------
bool MeshSet::mergeCoincidentContactPoints( double tolerance ) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->mergeCoincidentContactPoints(this->m_range, tolerance);
}

//----------------------------------------------------------------------------
//intersect two mesh sets, placing the results in the return mesh set
MeshSet set_intersect( const MeshSet& a, const MeshSet& b)
{
  if( a.m_parent != b.m_parent )
    { //return an empty MeshSet if the collections don't match
    return smtk::mesh::MeshSet(a.m_parent,
                               a.m_handle,
                               smtk::mesh::HandleRange());
    }

  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();
  smtk::mesh::HandleRange result = iface->rangeIntersect(a.m_range, b.m_range);
  return smtk::mesh::MeshSet(a.m_parent, a.m_handle, result);
}

//----------------------------------------------------------------------------
//subtract mesh b from a, placing the results in the return mesh set
MeshSet set_difference( const MeshSet& a, const MeshSet& b)
{
  if( a.m_parent != b.m_parent )
    { //return an empty MeshSet if the collections don't match
    return smtk::mesh::MeshSet(a.m_parent,
                               a.m_handle,
                               smtk::mesh::HandleRange());
    }

  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();
  smtk::mesh::HandleRange result = iface->rangeDifference(a.m_range, b.m_range);
  return smtk::mesh::MeshSet(a.m_parent, a.m_handle, result);
}

//----------------------------------------------------------------------------
//union two mesh sets, placing the results in the return mesh set
MeshSet set_union( const MeshSet& a, const MeshSet& b )
{
  if( a.m_parent != b.m_parent )
    { //return an empty MeshSet if the collections don't match
    return smtk::mesh::MeshSet(a.m_parent,
                               a.m_handle,
                               smtk::mesh::HandleRange());
    }

  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();
  smtk::mesh::HandleRange result = iface->rangeUnion(a.m_range, b.m_range);
  return smtk::mesh::MeshSet(a.m_parent, a.m_handle, result);
}

//----------------------------------------------------------------------------
SMTKCORE_EXPORT void for_each(const MeshSet& a, MeshForEach &filter)
{
  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();

  filter.m_collection=a.m_parent;
  iface->meshForEach(a.m_range, filter);
}


}
}
