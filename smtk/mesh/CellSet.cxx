//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/CellSet.h"
#include "smtk/mesh/Collection.h"

#include "smtk/mesh/Interface.h"
#include "smtk/mesh/ContainsFunctors.h"

namespace smtk {
namespace mesh {


//----------------------------------------------------------------------------
CellSet::CellSet(const smtk::mesh::CollectionPtr& parent,
                 const smtk::mesh::HandleRange& range):
  m_parent(parent),
  m_range(range) //range of moab cell entity ids
{

}

//----------------------------------------------------------------------------
CellSet::CellSet(const smtk::mesh::ConstCollectionPtr& parent,
                 const smtk::mesh::HandleRange& range):
  m_parent(std::const_pointer_cast<smtk::mesh::Collection>(parent)),
  m_range(range) //range of moab cell entity ids
{

}

//----------------------------------------------------------------------------
CellSet::CellSet(const smtk::mesh::CollectionPtr& parent,
                 const std::vector<smtk::mesh::Handle>& cellIds):
  m_parent(parent),
  m_range()
{
  std::copy(cellIds.rbegin(), cellIds.rend(),
            HandleRangeInserter(this->m_range));
}

//----------------------------------------------------------------------------
CellSet::CellSet(const smtk::mesh::CollectionPtr& parent,
                 const std::set<smtk::mesh::Handle>& cellIds):
  m_parent(parent),
  m_range()
{
  std::copy(cellIds.rbegin(), cellIds.rend(),
            HandleRangeInserter(this->m_range));
}

//----------------------------------------------------------------------------
CellSet::CellSet(const smtk::mesh::CellSet& other):
  m_parent(other.m_parent),
  m_range(other.m_range)
{

}

//----------------------------------------------------------------------------
CellSet::~CellSet()
{

}

//----------------------------------------------------------------------------
CellSet& CellSet::operator=(const CellSet& other)
{
  this->m_parent = other.m_parent;
  this->m_range = other.m_range;
  return *this;
}

//----------------------------------------------------------------------------
bool CellSet::operator==(const CellSet& other) const
{
  return this->m_parent == other.m_parent &&
         //empty is a fast way to check for easy mismatching ranges
         this->m_range.empty() == other.m_range.empty()  &&
         this->m_range == other.m_range;
}

//----------------------------------------------------------------------------
bool CellSet::operator!=(const CellSet& other) const
{
  return !(*this == other);
}

//----------------------------------------------------------------------------
bool CellSet::append( const CellSet& other)
{
  const bool can_append = this->m_parent == other.m_parent;
  if(can_append)
    {
    this->m_range.insert(other.m_range.begin(), other.m_range.end());
    }
  return can_append;
}

//----------------------------------------------------------------------------
bool CellSet::is_empty( ) const
{
  return this->m_range.empty();
}

//----------------------------------------------------------------------------
std::size_t CellSet::size( ) const
{
  return this->m_range.size();
}

//----------------------------------------------------------------------------
smtk::mesh::TypeSet CellSet::types() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->computeTypes( this->m_range );
}


//----------------------------------------------------------------------------
smtk::mesh::PointSet CellSet::points( ) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange range = iface->getPoints( this->m_range );
  return smtk::mesh::PointSet(this->m_parent, range );
}

//----------------------------------------------------------------------------
smtk::mesh::PointSet CellSet::points( std::size_t position ) const
{
  smtk::mesh::HandleRange singleIndex;
  singleIndex.insert(this->m_range[position]);

  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange range = iface->getPoints( singleIndex );
  return smtk::mesh::PointSet(this->m_parent, range );
}

//----------------------------------------------------------------------------
smtk::mesh::PointConnectivity CellSet::pointConnectivity( ) const
{
  return smtk::mesh::PointConnectivity(this->m_parent, this->m_range);
}

//----------------------------------------------------------------------------
smtk::mesh::PointConnectivity CellSet::pointConnectivity( std::size_t position ) const
{
  smtk::mesh::HandleRange singleIndex;
  singleIndex.insert(this->m_range[position]);
  return smtk::mesh::PointConnectivity(this->m_parent, singleIndex);
}

//----------------------------------------------------------------------------
/**\brief Get the parent collection that this meshset belongs to.
  *
  */
const smtk::mesh::CollectionPtr& CellSet::collection() const
{
  return this->m_parent;
}

//----------------------------------------------------------------------------
//intersect two mesh sets, placing the results in the return mesh set
CellSet set_intersect( const CellSet& a, const CellSet& b)
{
  if( a.m_parent != b.m_parent )
    { //return an empty CellSet if the collections don't match
    return smtk::mesh::CellSet(a.m_parent, smtk::mesh::HandleRange());
    }

  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();
  smtk::mesh::HandleRange result = iface->rangeIntersect(a.m_range, b.m_range);
  return smtk::mesh::CellSet(a.m_parent, result);
}

//----------------------------------------------------------------------------
//subtract mesh b from a, placing the results in the return mesh set
CellSet set_difference( const CellSet& a, const CellSet& b)
{
  if( a.m_parent != b.m_parent )
    { //return an empty CellSet if the collections don't match
    return smtk::mesh::CellSet(a.m_parent,
                               smtk::mesh::HandleRange());
    }

  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();
  smtk::mesh::HandleRange result = iface->rangeDifference(a.m_range, b.m_range);
  return smtk::mesh::CellSet(a.m_parent, result);
}

//----------------------------------------------------------------------------
//union two mesh sets, placing the results in the return mesh set
CellSet set_union( const CellSet& a, const CellSet& b )
{
  if( a.m_parent != b.m_parent )
    { //return an empty CellSet if the collections don't match
    return smtk::mesh::CellSet(a.m_parent,
                               smtk::mesh::HandleRange());
    }

  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();
  smtk::mesh::HandleRange result = iface->rangeUnion(a.m_range, b.m_range);
  return smtk::mesh::CellSet(a.m_parent, result);
}

//----------------------------------------------------------------------------
//intersect two cell sets at the point id level, all cells from b which
//share points with cells in a are placed in the resulting CellSet.
CellSet point_intersect( const CellSet& a, const CellSet& b, ContainmentType t)
{
  if( a.m_parent != b.m_parent )
    { //return an empty CellSet if the collections don't match
    return smtk::mesh::CellSet(a.m_parent, smtk::mesh::HandleRange());
    }

  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();
  smtk::mesh::PointConnectivity pc(b.m_parent, b.m_range);

  //switch the algorithm based on the containment type
  smtk::mesh::HandleRange result;
  if(t == smtk::mesh::PartiallyContained)
    {
    smtk::mesh::PartiallyContainedFunctor f;
    result = iface->pointIntersect(a.m_range, b.m_range, pc, f);
    }
  else
    {
    smtk::mesh::FullyContainedFunctor f;
    result = iface->pointIntersect(a.m_range, b.m_range, pc, f);
    }
  return smtk::mesh::CellSet(a.m_parent, result);
}

//----------------------------------------------------------------------------
//subtract two cell sets at the point id level, all cells from b whose
//points are not used by cells from a are placed in the resulting CellSet.
CellSet point_difference( const CellSet& a, const CellSet& b, ContainmentType t)
{
  if( a.m_parent != b.m_parent )
    { //return an empty CellSet if the collections don't match
    return smtk::mesh::CellSet(a.m_parent, smtk::mesh::HandleRange());
    }

  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();
  smtk::mesh::PointConnectivity pc(b.m_parent, b.m_range);

  //switch the algorithm based on the containment type
  smtk::mesh::HandleRange result;
  if(t == smtk::mesh::PartiallyContained)
    {
    smtk::mesh::PartiallyContainedFunctor f;
    result = iface->pointDifference(a.m_range, b.m_range, pc, f);
    }
  else
    {
    smtk::mesh::FullyContainedFunctor f;
    result = iface->pointDifference(a.m_range, b.m_range, pc, f);
    }

  return smtk::mesh::CellSet(a.m_parent, result);
}

//----------------------------------------------------------------------------
SMTKCORE_EXPORT void for_each(const CellSet& a, CellForEach &filter)
{
  smtk::mesh::PointConnectivity pc(a.m_parent, a.m_range);
  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();

  filter.collection(a.m_parent);
  iface->cellForEach(a.m_range, pc, filter);
}

}
}
