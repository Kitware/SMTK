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
#include "smtk/mesh/moab/Interface.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/QueryTypes.h"
#include "smtk/mesh/ContainsFunctors.h"

#include "smtk/mesh/moab/CellTypeToType.h"
#include "smtk/mesh/moab/Allocator.h"
#include "smtk/mesh/moab/PointConnectivityStorage.h"

#include "moab/Core.hpp"
#include "moab/FileOptions.hpp"
#include "moab/Interface.hpp"
#include "moab/ReaderIface.hpp"
#include "moab/Skinner.hpp"

#define BEING_INCLUDED_BY_INTERFACE_CXX
//required to go after moab includes
#include "smtk/mesh/moab/Tags.h"
#undef BEING_USED_BY_INTERFACE_CXX

#include <algorithm>
#include <cstring>
#include <set>

namespace smtk {
namespace mesh {
namespace moab {

namespace detail
{
//----------------------------------------------------------------------------
smtk::mesh::HandleRange vectorToRange(std::vector< ::moab::EntityHandle >& vresult)
{
  smtk::mesh::HandleRange resulting_range;
  smtk::mesh::HandleRange::iterator hint = resulting_range.begin();
  const std::size_t size = vresult.size();
  for(std::size_t i = 0; i < size;)
    {
    std::size_t j;
    for(j = i + 1; j < size && vresult[j] == 1 + vresult[j-1]; j++);
      //empty for loop
    hint = resulting_range.insert( hint, vresult[i], vresult[i] + (j-i-1) );
    i = j;
    }
  return resulting_range;
}

//----------------------------------------------------------------------------
template<typename T, typename U>
std::vector< T > computeDenseTagValues(U tag,
                                       const smtk::mesh::HandleRange& meshsets,
                                       ::moab::Interface* iface)
{
  std::vector< T > result;

  //fetch all entities with the given tag
  smtk::mesh::HandleRange entitiesWithTag;
  iface->get_entities_by_type_and_tag( iface->get_root_set(),
                                       ::moab::MBENTITYSET,
                                       tag.moabTagPtr(),
                                       NULL,
                                       1,
                                       entitiesWithTag);

  //we have all entity sets that have the this tag
  //now we need to find the subset that is part of our
  //HandleRange
  entitiesWithTag = ::moab::intersect(entitiesWithTag, meshsets);

  //return early if nothing has the tag.
  //this also makes it safer to derefence the std vector below
  if( entitiesWithTag.empty() )
    {
    return result;
    }

  //allocate a vector large enough to hold the tag values for every element
  std::vector< int > tag_values;
  tag_values.resize( entitiesWithTag.size() );
  void *tag_v_ptr = &tag_values[0];

  //fetch the tag for each item in the range in bulk
  iface->tag_get_data(tag.moabTag(),
                      entitiesWithTag,
                      tag_v_ptr);

  //find and remove duplicates
  std::sort( tag_values.begin(), tag_values.end() );
  tag_values.erase( std::unique( tag_values.begin(), tag_values.end() ),
                        tag_values.end() );

  //for each tag value convert it to a type T, where T is an
  //IntTag from smtk::mesh
  result.reserve( tag_values.size() );
  typedef std::vector< int >::const_iterator cit;
  for(cit i=tag_values.begin(); i != tag_values.end(); ++i)
    {
    result.push_back( T(*i) );
    }
  return result;
}

//----------------------------------------------------------------------------
template<typename T>
bool setDenseTagValues(T tag, const smtk::mesh::HandleRange& handles,
                       ::moab::Interface* iface)
{
  //create a vector the same value so we can assign a tag
  std::vector< int > values;
  values.resize(handles.size(), tag.value());
  const void *tag_v_ptr = &values[0];

  ::moab::ErrorCode rval = iface->tag_set_data(tag.moabTag(),
                                               handles,
                                               tag_v_ptr);
  return (rval == ::moab::MB_SUCCESS);
}

} //detail


//construct an empty interface instance
smtk::mesh::moab::InterfacePtr make_interface()
{
  //Core is a fully implemented moab::Interface
  return smtk::mesh::moab::InterfacePtr( new smtk::mesh::moab::Interface() );
}

//Given a smtk::mesh Interface convert it to a smtk::mesh::moab interface
smtk::mesh::moab::InterfacePtr extract_interface( const smtk::mesh::CollectionPtr& c)
{
  return smtk::dynamic_pointer_cast< smtk::mesh::moab::Interface > ( c->interface() );
}

//Given a smtk::mesh Interface convert it to a smtk::mesh::moab interface, and than
//extract the raw moab interface pointer from that
::moab::Interface *const extract_moab_interface( const smtk::mesh::InterfacePtr &iface)
{
  smtk::mesh::moab::Interface* mi = dynamic_cast< smtk::mesh::moab::Interface*>(iface.get());
  return (mi == NULL) ? NULL : mi->moabInterface();
}

//----------------------------------------------------------------------------
Interface::Interface():
  m_iface( new ::moab::Core() ),
  m_alloc( new smtk::mesh::moab::Allocator( this->m_iface.get() ) )
{

}

//----------------------------------------------------------------------------
Interface::~Interface()
{

}
//----------------------------------------------------------------------------
smtk::mesh::AllocatorPtr Interface::allocator()
{
  return this->m_alloc;
}

//----------------------------------------------------------------------------
smtk::mesh::Handle Interface::getRoot() const

{
  return m_iface->get_root_set();
}

//----------------------------------------------------------------------------
bool Interface::createMesh(const smtk::mesh::HandleRange& cells,
                           smtk::mesh::Handle& meshHandle)
{
  if(cells.empty())
    {
    return false;
    }

  //make sure the cells are actually cells instead of meshsets.
  //we currently don't want this allow adding sub meshsets
  if(cells.num_of_type(::moab::MBENTITYSET) != 0)
    {
    return false;
    }

  const unsigned int options = 0;
  ::moab::ErrorCode rval = m_iface->create_meshset( options , meshHandle );
  if(rval == ::moab::MB_SUCCESS)
    {
    m_iface->add_entities( meshHandle, cells );
    m_iface->add_parent_child( m_iface->get_root_set(),
                               meshHandle );

    int dimension = 4;
    bool hasDim = false;
    while(hasDim == false && dimension >= 0)
      {
      //by starting at 4 and decrementing we don't need to branch
      //on hasDim to see if we need to decrement at the end of
      //the while loop
      --dimension;

      //iterate the entities and find the higest dimension of cell.
      //once that is found add a geom sparse tag to the mesh
      hasDim = cells.num_of_dimension(dimension);
      }

    //add the dim tag
    tag::QueryDimTag dimTag(dimension, this->moabInterface());
    m_iface->tag_set_data( dimTag.moabTag(),
                           &meshHandle, 1,
                           dimTag.moabTagValuePtr());
    }
   return (rval == ::moab::MB_SUCCESS);
}

//----------------------------------------------------------------------------
std::size_t Interface::numMeshes(smtk::mesh::Handle handle) const
{
  int num_ents = 0;
  m_iface->get_number_entities_by_type( handle, ::moab::MBENTITYSET, num_ents);
  return static_cast<std::size_t>(num_ents);
}


//----------------------------------------------------------------------------
smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle) const

{
  smtk::mesh::HandleRange range;
  m_iface->get_entities_by_type(handle, ::moab::MBENTITYSET, range);
  return range;
}

//----------------------------------------------------------------------------
smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle,
                                                int dimension) const

{
  typedef std::vector< ::moab::EntityHandle >::const_iterator it;

  //use a vector since we are going to do single element iteration, and
  //removal.
  std::vector< ::moab::EntityHandle > all_ents;
  std::vector< ::moab::EntityHandle > matching_ents;
  m_iface->get_entities_by_type(handle, ::moab::MBENTITYSET, all_ents);

  //add all meshsets that have at least a single cell of the given dimension
  for( it i = all_ents.begin(); i != all_ents.end(); ++i )
    {
    smtk::mesh::HandleRange cells_of_given_dim;
    m_iface->get_entities_by_dimension(*i,dimension,cells_of_given_dim);
    if(!cells_of_given_dim.empty())
      {
      matching_ents.push_back(*i);
      }
    }
  return detail::vectorToRange(matching_ents);
}

//----------------------------------------------------------------------------
//find all entity sets that have this exact name tag
smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle,
                                                const std::string& name) const

{
  typedef std::vector< ::moab::EntityHandle >::const_iterator it;

  //use a vector since we are going to do single element iteration, and
  //removal.
  std::vector< ::moab::EntityHandle > all_ents;
  std::vector< ::moab::EntityHandle > matching_ents;
  m_iface->get_entities_by_type(handle, ::moab::MBENTITYSET, all_ents);

  //see which ones have a a matching name, and if so add it
  //we don't use get_entities_by_type_and_tag as it doesn't
  //seem to work with name tags
  tag::QueryNameTag query_name(this->moabInterface());
  for( it i = all_ents.begin(); i != all_ents.end(); ++i )
    {
    const bool has_name = query_name.fetch_name(*i);
    if(has_name &&
       ( std::strcmp(name.c_str(), query_name.current_name()) == 0 ) )
      { //has a matching name
      matching_ents.push_back(*i);
      }
    }
  return detail::vectorToRange(matching_ents);
}

//----------------------------------------------------------------------------
//find all entity sets that have this exact name tag
smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle,
                                               const smtk::mesh::Domain &domain) const

{
  tag::QueryMaterialTag mtag(domain.value(),this->moabInterface());

  smtk::mesh::HandleRange result;
  ::moab::ErrorCode rval;
  rval = m_iface->get_entities_by_type_and_tag(handle,
                                               ::moab::MBENTITYSET,
                                               mtag.moabTagPtr(),
                                               mtag.moabTagValuePtr(),
                                               1,
                                               result);
  if(rval != ::moab::MB_SUCCESS)
    {
    result.clear();
    }
  return result;
}

//----------------------------------------------------------------------------
//find all entity sets that have this exact name tag
smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle,
                                               const smtk::mesh::Dirichlet &dirichlet) const

{
  tag::QueryDirichletTag dtag(dirichlet.value(),this->moabInterface());

  smtk::mesh::HandleRange result;
  ::moab::ErrorCode rval;
  rval = m_iface->get_entities_by_type_and_tag(handle,
                                               ::moab::MBENTITYSET,
                                               dtag.moabTagPtr(),
                                               dtag.moabTagValuePtr(),
                                               1,
                                               result);
  if(rval != ::moab::MB_SUCCESS)
    {
    result.clear();
    }
  return result;
}

//----------------------------------------------------------------------------
//get all cells held by this range
smtk::mesh::HandleRange Interface::getCells(const HandleRange &meshsets) const

{
  // get all non-meshset entities in meshset, including in contained meshsets
  typedef smtk::mesh::HandleRange::const_iterator iterator;
  smtk::mesh::HandleRange entitiesCells;
  for(iterator i = meshsets.begin(); i != meshsets.end(); ++i)
    {
    //get_entities_by_handle appends to the range given
    m_iface->get_entities_by_handle(*i, entitiesCells, true);
    }
  return entitiesCells;
}


//----------------------------------------------------------------------------
//get all cells held by this range handle of a given cell type
smtk::mesh::HandleRange Interface::getCells(const HandleRange &meshsets,
                                             smtk::mesh::CellType cellType) const
{
  int moabCellType = smtk::mesh::moab::smtkToMOABCell(cellType);

  smtk::mesh::HandleRange entitiesCells;

  // get all non-meshset entities in meshset of a given cell type
  typedef smtk::mesh::HandleRange::const_iterator iterator;
  for(iterator i = meshsets.begin(); i != meshsets.end(); ++i)
    {
    //get_entities_by_type appends to the range given
    m_iface->get_entities_by_type(*i,
                                static_cast< ::moab::EntityType >(moabCellType),
                                entitiesCells,
                                true);
    }

  return entitiesCells;
}

//----------------------------------------------------------------------------
//get all cells held by this range handle of a given cell type(s)
smtk::mesh::HandleRange Interface::getCells(const smtk::mesh::HandleRange& meshsets,
                                             const smtk::mesh::CellTypes& cellTypes) const

{
  const std::size_t cellTypesToFind = cellTypes.count();
  if( cellTypesToFind == cellTypes.size())
    { //if all the cellTypes are enabled we should just use get_cells
      //all() method can't be used as it was added in C++11
    return this->getCells( meshsets );
    }
  else if(cellTypesToFind == 0)
    {
    return smtk::mesh::HandleRange();
    }

  //we now search from highest cell type to lowest cell type adding everything
  //to the range. The reason for this is that ranges perform best when inserting
  //from high to low values
  smtk::mesh::HandleRange entitiesCells;
  for(int i = (cellTypes.size() -1); i >= 0; --i )
    {
    //skip all cell types we don't have
    if( !cellTypes[i] )
      { continue; }

    smtk::mesh::CellType currentCellType = static_cast<smtk::mesh::CellType>(i);

    smtk::mesh::HandleRange cellEnts = this->getCells(meshsets, currentCellType);

    entitiesCells.insert(cellEnts.begin(), cellEnts.end());
    }

  return entitiesCells;
}

//----------------------------------------------------------------------------
//get all cells held by this range handle of a given dimension
smtk::mesh::HandleRange Interface::getCells(const smtk::mesh::HandleRange& meshsets,
                                            smtk::mesh::DimensionType dim) const

{
  const int dimension = static_cast<int>(dim);

  //get all non-meshset entities of a given dimension
  typedef smtk::mesh::HandleRange::const_iterator iterator;
  smtk::mesh::HandleRange entitiesCells;
  for(iterator i = meshsets.begin(); i != meshsets.end(); ++i)
    {
    //get_entities_by_dimension appends to the range given
    m_iface->get_entities_by_dimension(*i, dimension, entitiesCells, true);
    }
  return entitiesCells;
}

//----------------------------------------------------------------------------
std::vector< std::string > Interface::computeNames(const smtk::mesh::HandleRange& meshsets) const
{
  //construct a name tag query helper class
  tag::QueryNameTag query_name(this->moabInterface());

  typedef smtk::mesh::HandleRange::const_iterator it;
  std::set< std::string > unique_names;
  for(it i = meshsets.begin(); i != meshsets.end(); ++i)
    {
    const bool has_name = query_name.fetch_name(*i);
    if(has_name)
      {
      unique_names.insert( std::string(query_name.current_name()) );
      }
    }
  //return a vector of the unique names
  return std::vector< std::string >(unique_names.begin(), unique_names.end());
}

//----------------------------------------------------------------------------
std::vector< smtk::mesh::Domain > Interface::computeDomainValues(const smtk::mesh::HandleRange& meshsets) const
{
  tag::QueryMaterialTag mtag(0,this->moabInterface());
  return detail::computeDenseTagValues<smtk::mesh::Domain>(mtag,
                                                             meshsets,
                                                             this->moabInterface());
}


//----------------------------------------------------------------------------
std::vector< smtk::mesh::Dirichlet > Interface::computeDirichletValues(const smtk::mesh::HandleRange& meshsets) const
{
  tag::QueryDirichletTag dtag(0,this->moabInterface());
  return detail::computeDenseTagValues<smtk::mesh::Dirichlet>(dtag,
                                                              meshsets,
                                                              this->moabInterface());
}

//----------------------------------------------------------------------------
smtk::mesh::TypeSet Interface::computeTypes(smtk::mesh::Handle handle) const
{
  int numMeshes = 0;
  m_iface->get_number_entities_by_type( handle, ::moab::MBENTITYSET, numMeshes);

  //iterate over all the celltypes and get the number for each
  //construct a smtk::mesh::CellTypes at the same time
  typedef ::smtk::mesh::CellType CellEnum;
  smtk::mesh::CellTypes ctypes;
  if(numMeshes > 0)
    {
    for(int i=0; i < ctypes.size(); ++i )
      {
      CellEnum ce = static_cast<CellEnum>(i);
      //now we need to convert from CellEnum to MoabType
      int moabEType = smtk::mesh::moab::smtkToMOABCell(ce);

      //some of the cell types that smtk supports moab doesn't support
      //so we can't query on those.
      int num = 0;
      m_iface->get_number_entities_by_type(handle,
                                         static_cast< ::moab::EntityType >(moabEType),
                                         num);
      ctypes[ce] = (num > 0);
      }
    }

  //determine the state of the typeset
  const bool hasMeshes = numMeshes > 0;
  const bool hasCells = ctypes.any();
  return smtk::mesh::TypeSet(ctypes, hasMeshes, hasCells) ;
}

//----------------------------------------------------------------------------
bool Interface::computeShell(const smtk::mesh::HandleRange& meshes,
                             smtk::mesh::HandleRange& shell) const
{
  //step 1 get all the highest dimension cells for the meshes
  smtk::mesh::HandleRange cells;
  int dimension = 4;
  bool hasCells = false;
  while(hasCells == false && dimension >= 0)
    {
    --dimension;
    cells = this->getCells(meshes, static_cast<smtk::mesh::DimensionType>(dimension));
    hasCells = !cells.empty();
    }

  ::moab::Skinner skinner(this->moabInterface());
  ::moab::ErrorCode rval;
  rval = skinner.find_skin(this->getRoot(),
                           cells,
                           false, //return cells not verts
                           shell);
  return (rval == ::moab::MB_SUCCESS);
}

//----------------------------------------------------------------------------
bool Interface::setDomain(const smtk::mesh::HandleRange& meshsets,
                            const smtk::mesh::Domain& domain) const
{
  if(meshsets.empty())
    {
    return true;
    }

  tag::QueryMaterialTag mtag(domain.value(),this->moabInterface());
  return detail::setDenseTagValues(mtag,meshsets,this->moabInterface());
}

//----------------------------------------------------------------------------
bool Interface::setDirichlet(const smtk::mesh::HandleRange& meshsets,
                             const smtk::mesh::Dirichlet& dirichlet) const
{
  if(meshsets.empty())
    {
    return true;
    }

  tag::QueryDirichletTag dtag(dirichlet.value(),this->moabInterface());

  smtk::mesh::HandleRange cells = this->getCells(meshsets, smtk::mesh::Dims0);
  bool cellsTagged = detail::setDenseTagValues(dtag,cells,this->moabInterface());
  bool meshesTagged =detail::setDenseTagValues(dtag,meshsets,this->moabInterface());
  return cellsTagged && meshesTagged;
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
  if(a.empty() || b.empty())
    { //the intersection with nothing is nothing
    return smtk::mesh::HandleRange();
    }

  //first get all the points of a
  smtk::mesh::HandleRange a_points = a.subset_by_type( ::moab::MBVERTEX );
  m_iface->get_connectivity(a, a_points);

  if(a_points.empty())
    {
    return  smtk::mesh::HandleRange();
    }

  typedef smtk::mesh::HandleRange::const_iterator cit;
  std::vector< ::moab::EntityHandle > vresult;
  if(!bpc.is_empty())
    {
    int size=0;
    const smtk::mesh::Handle* connectivity;
    bpc.initCellTraversal();
    for(cit i = b.begin(); i!= b.end(); ++i)
      {
      const bool validCell = bpc.fetchNextCell(size, connectivity);
      if(validCell)
        {
        const bool contains = containsFunctor(a_points, connectivity, size);
        if(contains)
          { vresult.push_back( *i ); }
        }
      }
    }
  return detail::vectorToRange(vresult);
}

//----------------------------------------------------------------------------
smtk::mesh::HandleRange Interface::pointDifference(const smtk::mesh::HandleRange& a,
                                                   const smtk::mesh::HandleRange& b,
                                                   smtk::mesh::PointConnectivity& bpc,
                                                   const smtk::mesh::ContainsFunctor& containsFunctor) const
{
  if(a.empty() || b.empty())
    { //the intersection with nothing is nothing
    return smtk::mesh::HandleRange();
    }

  //first get all the points of a
  smtk::mesh::HandleRange a_points = a.subset_by_type( ::moab::MBVERTEX );
  m_iface->get_connectivity(a, a_points);

  if(a_points.empty())
    {
    return  smtk::mesh::HandleRange();
    }

  typedef smtk::mesh::HandleRange::const_iterator cit;
  std::vector< ::moab::EntityHandle > vresult;
  if(!bpc.is_empty())
    {
    int size=0;
    const smtk::mesh::Handle* connectivity;
    bpc.initCellTraversal();
    for(cit i = b.begin(); i!= b.end(); ++i)
      {
      const bool validCell = bpc.fetchNextCell(size, connectivity);
      if(validCell)
        {
        const bool contains = containsFunctor(a_points, connectivity, size);
        if(!contains)
          { vresult.push_back( *i ); }
        }
      }
    }
  return detail::vectorToRange(vresult);
}

//----------------------------------------------------------------------------
void Interface::cellForEach(const smtk::mesh::HandleRange &cells,
                            smtk::mesh::PointConnectivity& pc,
                            smtk::mesh::CellForEach& filter) const
{
  if(!pc.is_empty())
    {
    int size=0;
    const smtk::mesh::Handle* points;
    std::vector<double> coords;

    typedef smtk::mesh::HandleRange::const_iterator cit;
    pc.initCellTraversal();
    for(cit i = cells.begin(); i!= cells.end(); ++i)
      {
      bool validCell = pc.fetchNextCell(size, points);
      if(!validCell)
        {
        continue;
        }

      coords.reserve(size*3);

      //query to grab the coordinates for these points
      m_iface->get_coords(points,
                          size,
                          &coords[0]);

      //call the custom filter
      filter(size,points,&coords[0]);
      }
    }
  return;
}

//----------------------------------------------------------------------------
void Interface::meshForEach(const smtk::mesh::HandleRange &meshes,
                            smtk::mesh::MeshForEach& filter) const
{
  if(!meshes.empty())
    {
    typedef smtk::mesh::HandleRange::const_iterator cit;
    for(cit i = meshes.begin(); i!= meshes.end(); ++i)
      {

      smtk::mesh::HandleRange singlHandle(*i,*i);
      smtk::mesh::MeshSet singleMesh(filter.m_collection,*i,singlHandle);

      //call the custom filter
      filter(singleMesh);
      }
    }
  return;
}

//----------------------------------------------------------------------------
bool Interface::deleteHandles(const smtk::mesh::HandleRange& toDel)
{
  //step 1. verify HandleRange isnt empty
  if(toDel.empty())
    {
    return true;
    }

  //step 2. verify HandleRange doesn't contain root Handle
  if(toDel.front() == this->getRoot())
    {
    //Ranges are always sorted, and the root is always id 0
    return false;
    }

  //step 3. verify HandleRange is either all entity sets or cells/verts
  //this could be a performance bottleneck since we are using size
  if(toDel.all_of_type(::moab::MBENTITYSET))
    {
    //we are all moab entity sets, fine to delete
    const::moab::ErrorCode rval = m_iface->delete_entities(toDel);
    return (rval == ::moab::MB_SUCCESS);
    }
  else if(toDel.num_of_type(::moab::MBENTITYSET) == 0)
    {
    //we have zero entity sets so we must be all cells/coords
    const::moab::ErrorCode rval = m_iface->delete_entities(toDel);
    return (rval == ::moab::MB_SUCCESS);
    }

  //we are mixed cells and entity sets and must fail
  return false;

}

//----------------------------------------------------------------------------
::moab::Interface *const Interface::moabInterface() const
{
  return this->m_iface.get();
}

}
}
}
