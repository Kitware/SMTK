//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/PointSet.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Interface.h"

namespace smtk {
namespace mesh {

//----------------------------------------------------------------------------
PointSet::PointSet(const smtk::mesh::CollectionPtr& parent,
                 const smtk::mesh::HandleRange& points):
  m_parent(parent),
  m_points(points)
{

}

//----------------------------------------------------------------------------
PointSet::PointSet(const smtk::mesh::PointSet& other):
  m_parent(other.m_parent),
  m_points(other.m_points)
{

}

//----------------------------------------------------------------------------
PointSet::~PointSet()
{

}

//----------------------------------------------------------------------------
PointSet& PointSet::operator=(const PointSet& other)
{
  this->m_parent = other.m_parent;
  this->m_points = other.m_points;
  return *this;
}

//----------------------------------------------------------------------------
bool PointSet::operator==(const PointSet& other) const
{
  return this->m_parent == other.m_parent && this->m_points == other.m_points;
}

//----------------------------------------------------------------------------
bool PointSet::operator!=(const PointSet& other) const
{
  return !(*this == other);
}

//----------------------------------------------------------------------------
bool PointSet::is_empty( ) const
{
  return this->m_points.empty();
}

//----------------------------------------------------------------------------
std::size_t PointSet::size( ) const
{
  return this->m_points.size();
}

//----------------------------------------------------------------------------
std::size_t PointSet::numberOfPointSet( ) const
{
  return this->m_points.size();
}


//----------------------------------------------------------------------------
bool PointSet::contains( const smtk::mesh::Handle& pointId ) const
{
  return this->m_points.find( pointId ) != this->m_points.end();
}

//----------------------------------------------------------------------------
std::size_t PointSet::find( const smtk::mesh::Handle& pointId ) const
{
  //yes index() method returns an int
  int index = this->m_points.index(pointId);
  if (index >= 0 )
    {
    return static_cast< std::size_t > (index);
    }
  return this->m_points.size();
}

//----------------------------------------------------------------------------
bool PointSet::get(double* xyz) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->getCoordinates(this->m_points, xyz);
}

//----------------------------------------------------------------------------
bool PointSet::get(float* xyz) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->getCoordinates(this->m_points, xyz);
}

//----------------------------------------------------------------------------
PointSet set_intersect( const PointSet& a, const PointSet& b )
{
  if( a.m_parent != b.m_parent )
    { //return an empty PointSet if the collections don't match
    return smtk::mesh::PointSet(a.m_parent,
                               smtk::mesh::HandleRange());
    }


  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();
  smtk::mesh::HandleRange result = iface->rangeIntersect(a.m_points, b.m_points);
  return smtk::mesh::PointSet(a.m_parent, result);
}

//----------------------------------------------------------------------------
PointSet set_difference( const PointSet& a, const PointSet& b )
{
  if( a.m_parent != b.m_parent )
    { //return an empty PointSet if the collections don't match
    return smtk::mesh::PointSet(a.m_parent,
                               smtk::mesh::HandleRange());
    }

  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();
  smtk::mesh::HandleRange result = iface->rangeDifference(a.m_points, b.m_points);
  return smtk::mesh::PointSet(a.m_parent, result);
}

//----------------------------------------------------------------------------
PointSet set_union( const PointSet& a, const PointSet& b )
{
  if( a.m_parent != b.m_parent )
    { //return an empty PointSet if the collections don't match
    return smtk::mesh::PointSet(a.m_parent, smtk::mesh::HandleRange());
    }

  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();
  smtk::mesh::HandleRange result = iface->rangeUnion(a.m_points, b.m_points);
  return smtk::mesh::PointSet(a.m_parent, result);
}

//----------------------------------------------------------------------------
void for_each( const PointSet& a, PointForEach& filter )
{
  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();
  filter.m_collection=a.m_parent;
  iface->pointForEach(a.m_points, filter);
}

}
}
