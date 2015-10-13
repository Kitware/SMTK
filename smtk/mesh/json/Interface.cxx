//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/mesh/json/Interface.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/QueryTypes.h"
#include "smtk/mesh/ContainsFunctors.h"

#include "smtk/mesh/moab/CellTypeToType.h"

#include <algorithm>
#include <cstring>
#include <set>

namespace smtk {
namespace mesh {
namespace json {

//construct an empty interface instance
smtk::mesh::json::InterfacePtr make_interface()
{
  return smtk::mesh::json::InterfacePtr( new smtk::mesh::json::Interface() );
}

//----------------------------------------------------------------------------
Interface::Interface():
  m_meshInfo()
{

}

//----------------------------------------------------------------------------
Interface::Interface( const std::vector<smtk::mesh::json::MeshInfo>& info ):
  m_meshInfo(info)
{

}

//----------------------------------------------------------------------------
Interface::~Interface()
{

}

//----------------------------------------------------------------------------
void Interface::addMeshes( const std::vector<smtk::mesh::json::MeshInfo>& info )
{
  this->m_meshInfo.insert(this->m_meshInfo.end(),
                          info.begin(),
                          info.end());
}

//----------------------------------------------------------------------------
smtk::mesh::AllocatorPtr Interface::allocator()
{
  return smtk::mesh::AllocatorPtr();
}

//----------------------------------------------------------------------------
smtk::mesh::ConnectivityStoragePtr Interface::connectivityStorage(
                                      const smtk::mesh::HandleRange& cells)
{
  //make boost shared_ptr
  smtk::mesh::ConnectivityStoragePtr cs;
  return cs;
}


//----------------------------------------------------------------------------
smtk::mesh::Handle Interface::getRoot() const

{
  return smtk::mesh::Handle(0);
}

//----------------------------------------------------------------------------
bool Interface::createMesh(const smtk::mesh::HandleRange& cells,
                           smtk::mesh::Handle& meshHandle)
{
  //this interface can't create new meshes
  return false;
}

//----------------------------------------------------------------------------
std::size_t Interface::numMeshes(smtk::mesh::Handle handle) const
{
  if(handle != this->getRoot())
    {
    return 0;
    }
  return this->m_meshInfo.size();
}


//----------------------------------------------------------------------------
smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle) const

{
  if(handle != this->getRoot())
    {
    return smtk::mesh::HandleRange();
    }

  smtk::mesh::HandleRange meshes;
  MeshInfoVecType::const_iterator i;
  for(i=this->m_meshInfo.begin(); i != this->m_meshInfo.end(); ++i)
    {
    meshes.insert(i->mesh());
    }
  return meshes;
}

//----------------------------------------------------------------------------
smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle,
                                                int dimension) const

{
  if(handle != this->getRoot())
    {
    return smtk::mesh::HandleRange();
    }

  const smtk::mesh::DimensionType dim =
                      static_cast<smtk::mesh::DimensionType>(dimension);

  smtk::mesh::HandleRange meshes;
  MeshInfoVecType::const_iterator i;
  for(i=this->m_meshInfo.begin(); i != this->m_meshInfo.end(); ++i)
    {
    if(i->types().hasDimension(dim))
      {
      meshes.insert(i->mesh());
      }
    }
  return meshes;
}

//----------------------------------------------------------------------------
//find all entity sets that have this exact name tag
smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle,
                                               const std::string& name) const

{
  return smtk::mesh::HandleRange();
}

//----------------------------------------------------------------------------
//find all entity sets that have this exact name tag
smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle,
                                               const smtk::mesh::Domain &domain) const

{
  if(handle != this->getRoot())
    {
    return smtk::mesh::HandleRange();
    }

  smtk::mesh::HandleRange meshes;
  MeshInfoVecType::const_iterator i;
  for(i=this->m_meshInfo.begin(); i != this->m_meshInfo.end(); ++i)
    {
    if(i->has(domain))
      {
      meshes.insert(i->mesh());
      }
    }
  return meshes;
}

//----------------------------------------------------------------------------
//find all entity sets that have this exact name tag
smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle,
                                               const smtk::mesh::Dirichlet &dirichlet) const

{
  smtk::mesh::HandleRange meshes;
  MeshInfoVecType::const_iterator i;
  for(i=this->m_meshInfo.begin(); i != this->m_meshInfo.end(); ++i)
    {
    if(i->has(dirichlet))
      {
      meshes.insert(i->mesh());
      }
    }
  return meshes;
}

//----------------------------------------------------------------------------
//find all entity sets that have this exact name tag
smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle,
                                               const smtk::mesh::Neumann &neumann) const

{
  smtk::mesh::HandleRange meshes;
  MeshInfoVecType::const_iterator i;
  for(i=this->m_meshInfo.begin(); i != this->m_meshInfo.end(); ++i)
    {
    if(i->has(neumann))
      {
      meshes.insert(i->mesh());
      }
    }
  return meshes;
}

//----------------------------------------------------------------------------
//get all cells held by this range
smtk::mesh::HandleRange Interface::getCells(const HandleRange &meshsets) const

{
  smtk::mesh::HandleRange cells;
  smtk::mesh::HandleRange::const_iterator i;
  for(i=meshsets.begin(); i != meshsets.end(); ++i)
    {
    MeshInfoVecType::const_iterator m = this->find(*i);
    if(m != this->m_meshInfo.end())
      {
      cells.merge(m->cells());
      }
    }
  return cells;
}


//----------------------------------------------------------------------------
//get all cells held by this range handle of a given cell type
smtk::mesh::HandleRange Interface::getCells(const HandleRange &meshsets,
                                            smtk::mesh::CellType cellType) const
{
  smtk::mesh::HandleRange cells;
  smtk::mesh::HandleRange::const_iterator i;
  for(i=meshsets.begin(); i != meshsets.end(); ++i)
    {
    MeshInfoVecType::const_iterator m = this->find(*i);
    if(m != this->m_meshInfo.end())
      {
      cells.merge(m->cells(cellType));
      }
    }
  return cells;
}

//----------------------------------------------------------------------------
//get all cells held by this range handle of a given cell type(s)
smtk::mesh::HandleRange Interface::getCells(const smtk::mesh::HandleRange& meshsets,
                                            const smtk::mesh::CellTypes& cellTypes) const

{
  smtk::mesh::HandleRange cells;
  smtk::mesh::HandleRange::const_iterator i;
  for(i=meshsets.begin(); i != meshsets.end(); ++i)
    {
    MeshInfoVecType::const_iterator m = this->find(*i);
    if(m != this->m_meshInfo.end())
      {
      cells.merge(m->cells(cellTypes));
      }
    }
  return cells;
}

//----------------------------------------------------------------------------
//get all cells held by this range handle of a given dimension
smtk::mesh::HandleRange Interface::getCells(const smtk::mesh::HandleRange& meshsets,
                                            smtk::mesh::DimensionType dim) const

{
  smtk::mesh::HandleRange cells;
  smtk::mesh::HandleRange::const_iterator i;
  for(i=meshsets.begin(); i != meshsets.end(); ++i)
    {
    MeshInfoVecType::const_iterator m = this->find(*i);
    if(m != this->m_meshInfo.end())
      {
      cells.merge(m->cells(dim));
      }
    }
  return cells;
}

//----------------------------------------------------------------------------
//get all cells held by this range handle of a given dimension
smtk::mesh::HandleRange Interface::getPoints(const smtk::mesh::HandleRange& cells) const

{
  //we need to break down the cells we are given and figure out the type for each

  //than call CN::VerticesPerEntity for each type

  //make a fake handleRange of the correct length
  return cells;
}

//----------------------------------------------------------------------------
bool Interface::getCoordinates(const smtk::mesh::HandleRange& points,
                               double* xyz) const

{
  return false;
}

//----------------------------------------------------------------------------
bool Interface::getCoordinates(const smtk::mesh::HandleRange& points,
                               float* xyz) const

{
  return false;
}


//----------------------------------------------------------------------------
std::vector< std::string > Interface::computeNames(const smtk::mesh::HandleRange& meshsets) const
{
  return std::vector< std::string >();
}

//----------------------------------------------------------------------------
std::vector< smtk::mesh::Domain > Interface::computeDomainValues(const smtk::mesh::HandleRange& meshsets) const
{
  std::set< smtk::mesh::Domain > domains;
  smtk::mesh::HandleRange::const_iterator i;
  for(i=meshsets.begin(); i != meshsets.end(); ++i)
    {
    MeshInfoVecType::const_iterator m = this->find(*i);
    const std::vector< smtk::mesh::Domain >& t = m->domains();
    domains.insert(t.begin(), t.end() );
    }

  //return a vector of all the unique domains we have
  return std::vector< smtk::mesh::Domain >( domains.begin(), domains.end() );
}


//----------------------------------------------------------------------------
std::vector< smtk::mesh::Dirichlet > Interface::computeDirichletValues(const smtk::mesh::HandleRange& meshsets) const
{
  std::set< smtk::mesh::Dirichlet > boundary;
  smtk::mesh::HandleRange::const_iterator i;
  for(i=meshsets.begin(); i != meshsets.end(); ++i)
    {
    MeshInfoVecType::const_iterator m = this->find(*i);
    const std::vector< smtk::mesh::Dirichlet >& t = m->dirichlets();
    boundary.insert(t.begin(), t.end() );
    }

  //return a vector of all the unique Dirichlet we have
  return std::vector< smtk::mesh::Dirichlet >( boundary.begin(), boundary.end() );
}

//----------------------------------------------------------------------------
std::vector< smtk::mesh::Neumann > Interface::computeNeumannValues(const smtk::mesh::HandleRange& meshsets) const
{
  std::set< smtk::mesh::Neumann > boundary;
  smtk::mesh::HandleRange::const_iterator i;
  for(i=meshsets.begin(); i != meshsets.end(); ++i)
    {
    MeshInfoVecType::const_iterator m = this->find(*i);
    const std::vector< smtk::mesh::Neumann >& t = m->neumanns();
    boundary.insert(t.begin(), t.end() );
    }

  //return a vector of all the unique Neumann we have
  return std::vector< smtk::mesh::Neumann >( boundary.begin(), boundary.end() );
}

//----------------------------------------------------------------------------
/**\brief Return the set of all UUIDs set on all entities in the meshsets.
  *
  */
smtk::common::UUIDArray Interface::computeModelEntities(const smtk::mesh::HandleRange& meshsets) const
{
  smtk::common::UUIDArray uuids;
  smtk::mesh::HandleRange::const_iterator i;
  for(i=meshsets.begin(); i != meshsets.end(); ++i)
    {
    MeshInfoVecType::const_iterator m = this->find(*i);
    const smtk::common::UUIDArray& t = m->modelUUIDS();
    if(t.size() > 0)
      {
      uuids.insert(uuids.end(), t.begin(), t.end());
      }
    }
  return uuids;
}

//----------------------------------------------------------------------------
smtk::mesh::TypeSet Interface::computeTypes(const smtk::mesh::HandleRange& range) const
{
  typedef smtk::mesh::HandleRange::const_iterator cit;
  typedef ::smtk::mesh::CellType CellEnum;


  smtk::mesh::HandleRange meshes = range.subset_by_type( ::moab::MBENTITYSET );
  smtk::mesh::HandleRange cells = ::moab::subtract(range,meshes);

  smtk::mesh::TypeSet result;
  smtk::mesh::HandleRange::const_iterator i;
  for(i=meshes.begin(); i != meshes.end(); ++i)
    {
    MeshInfoVecType::const_iterator m = this->find(*i);
    result += m->types();
    }

  smtk::mesh::CellTypes ctypes;

  //compute the type of the cells if we have any
  if(!cells.empty())
    {
    for (std::size_t i = 0; i < ctypes.size(); ++i )
      {
      //now we need to convert from CellEnum to MoabType
      const CellEnum ce = static_cast<CellEnum>(i);
      const ::moab::EntityType moabEType =
        static_cast< ::moab::EntityType >(smtk::mesh::moab::smtkToMOABCell(ce));

      //if num_of_type is greater than zero we have cells of that type
      if( cells.num_of_type( moabEType ) > 0) { ctypes[ce] = true; }
      }
    }

  const bool hasM = !(meshes.empty());
  const bool hasC = ctypes.any();
  smtk::mesh::TypeSet cellResult( ctypes, hasM, hasC );

  result += cellResult;
  return result;
}

//----------------------------------------------------------------------------
bool Interface::computeShell(const smtk::mesh::HandleRange& meshes,
                             smtk::mesh::HandleRange& shell) const
{
  return false;
 }

//----------------------------------------------------------------------------
bool Interface::mergeCoincidentContactPoints(const smtk::mesh::HandleRange& meshes,
                                            double tolerance) const
{
  return false;
}

//----------------------------------------------------------------------------
bool Interface::setDomain(const smtk::mesh::HandleRange& meshsets,
                            const smtk::mesh::Domain& domain) const
{
  return false;
}

//----------------------------------------------------------------------------
bool Interface::setDirichlet(const smtk::mesh::HandleRange& meshsets,
                             const smtk::mesh::Dirichlet& dirichlet) const
{
  return false;
}

//----------------------------------------------------------------------------
bool Interface::setNeumann(const smtk::mesh::HandleRange& meshsets,
                           const smtk::mesh::Neumann& neumann) const
{
  return false;
}

//----------------------------------------------------------------------------
/**\brief Set the model entity assigned to each meshset member to \a ent.
  */
bool Interface::setModelEntity(
  const smtk::mesh::HandleRange& meshsets,
  const smtk::common::UUID& uuid) const
{
  return false;
}

//----------------------------------------------------------------------------
/**\brief Find mesh entities associated with the given model entity.
  *
  */
smtk::mesh::HandleRange Interface::findAssociations(
  const smtk::mesh::Handle& root,
  const smtk::common::UUID& modelUUID)
{
  smtk::mesh::HandleRange result;
  if (!modelUUID)
    return result;

  smtk::mesh::HandleRange meshes;
  MeshInfoVecType::const_iterator i;
  for(i=this->m_meshInfo.begin(); i != this->m_meshInfo.end(); ++i)
    {
    const smtk::common::UUIDArray& uuids = i->modelUUIDS();
    const bool contains = std::find(uuids.begin(), uuids.end(), modelUUID) != uuids.end();
    if( contains )
      {
      meshes.insert(i->mesh());
      }
    }
  return meshes;
}

//----------------------------------------------------------------------------
bool Interface::addAssociation(const smtk::common::UUID& modelUUID,
                               const smtk::mesh::HandleRange& range)
{
  return false;
}

//----------------------------------------------------------------------------
smtk::mesh::HandleRange Interface::rangeIntersect(const smtk::mesh::HandleRange& a,
                                                 const smtk::mesh::HandleRange& b) const
{
  return ::moab::intersect(a,b);
}

//----------------------------------------------------------------------------
smtk::mesh::HandleRange Interface::rangeDifference(const smtk::mesh::HandleRange& a,
                                                   const smtk::mesh::HandleRange& b) const
{
  return ::moab::subtract(a,b);
}

//----------------------------------------------------------------------------
smtk::mesh::HandleRange Interface::rangeUnion(const smtk::mesh::HandleRange& a,
                                              const smtk::mesh::HandleRange& b) const
{
  return ::moab::unite(a,b);
}

//----------------------------------------------------------------------------
smtk::mesh::HandleRange Interface::pointIntersect(const smtk::mesh::HandleRange& a,
                                                  const smtk::mesh::HandleRange& b,
                                                  smtk::mesh::PointConnectivity& bpc,
                                                  const smtk::mesh::ContainsFunctor& containsFunctor) const
{
  return smtk::mesh::HandleRange();
}

//----------------------------------------------------------------------------
smtk::mesh::HandleRange Interface::pointDifference(const smtk::mesh::HandleRange& a,
                                                   const smtk::mesh::HandleRange& b,
                                                   smtk::mesh::PointConnectivity& bpc,
                                                   const smtk::mesh::ContainsFunctor& containsFunctor) const
{
  return smtk::mesh::HandleRange();
}

//----------------------------------------------------------------------------
void Interface::pointForEach(const HandleRange &points,
                             smtk::mesh::PointForEach& filter) const
{
  (void) points;
  (void) filter;
}

//----------------------------------------------------------------------------
void Interface::cellForEach(smtk::mesh::PointConnectivity& pc,
                            smtk::mesh::CellForEach& filter) const
{
  (void) pc;
  (void) filter;
}

//----------------------------------------------------------------------------
void Interface::meshForEach(const smtk::mesh::HandleRange &meshes,
                            smtk::mesh::MeshForEach& filter) const
{
  (void) meshes;
  (void) filter;
}

//----------------------------------------------------------------------------
bool Interface::deleteHandles(const smtk::mesh::HandleRange& toDel)
{
  return false;
}

//----------------------------------------------------------------------------
Interface::MeshInfoVecType::const_iterator
Interface::find(smtk::mesh::Handle handle) const
{
  MeshInfoVecType::const_iterator result =
          std::find(this->m_meshInfo.begin(), this->m_meshInfo.end(), handle);
  return result;
}

}
}
}
