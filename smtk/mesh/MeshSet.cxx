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
smtk::mesh::CellSet MeshSet::cells( )
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells( this->m_range );
  return smtk::mesh::CellSet(this->m_parent, range);
}

//----------------------------------------------------------------------------
smtk::mesh::Points MeshSet::points( )
{
  return smtk::mesh::Points();
}

//----------------------------------------------------------------------------
smtk::mesh::PointConnectivity MeshSet::pointConnectivity( )
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells( this->m_range );
  return smtk::mesh::PointConnectivity(this->m_parent, range);
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet MeshSet::cells( smtk::mesh::CellType cellType )
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells( this->m_range, cellType );
  return smtk::mesh::CellSet(this->m_parent, range);
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet MeshSet::cells( smtk::mesh::CellTypes cellTypes )
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells( this->m_range, cellTypes );
  return smtk::mesh::CellSet(this->m_parent, range);
}

//----------------------------------------------------------------------------
smtk::mesh::CellSet MeshSet::cells( smtk::mesh::DimensionType dim )
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells( this->m_range, dim );
  return smtk::mesh::CellSet(this->m_parent, range);
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


}
}
