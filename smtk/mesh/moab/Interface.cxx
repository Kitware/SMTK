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
#include "smtk/mesh/moab/ConnectivityStorage.h"
#include "smtk/mesh/moab/MergeMeshVertices.h"

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
std::vector< T > computeDenseIntTagValues(U tag,
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
template<typename T, typename U>
T computeDenseOpaqueTagValues(
  U tag,
  const smtk::mesh::HandleRange& meshsets,
  ::moab::Interface* iface)
{
  T result;

  // Fetch all entities with the given tag
  smtk::mesh::HandleRange entitiesWithTag;
  iface->get_entities_by_type_and_tag(
    iface->get_root_set(), ::moab::MBENTITYSET,
    tag.moabTagPtr(), NULL, 1,
    entitiesWithTag);

  // We have all entity sets that have the this tag; now we need
  // to find the subset that is part of our HandleRange.
  entitiesWithTag = ::moab::intersect(entitiesWithTag, meshsets);

  // Return early if nothing has the tag.
  // This also makes it safer to derefence the std vector below
  if( entitiesWithTag.empty() )
    return result;

  std::vector<unsigned char> tag_values;
  tag_values.resize(entitiesWithTag.size() * tag.size());
  void* tag_v_ptr = &tag_values[0];

  // Fetch the tag for each item in the range in bulk
  iface->tag_get_data(tag.moabTag(), entitiesWithTag, tag_v_ptr);

  // For each tag value convert it to a value type for the
  // output container T.
  typedef std::vector<unsigned char>::const_iterator cit;
  for(cit i = tag_values.begin(); i != tag_values.end(); i += tag.size())
    {
    result.push_back(typename T::value_type(&(*i), &(*i) + tag.size()));
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

template<typename T>
bool setDenseOpaqueTagValues(T tag, const smtk::mesh::HandleRange& handles,
                       ::moab::Interface* iface)
{
  //create a vector the same value so we can assign a tag
  std::vector<unsigned char> values;
  values.resize(handles.size() * tag.size());
  for (std::size_t i = 0; i < handles.size(); ++i)
    memcpy(&values[i * tag.size()], tag.value(), tag.size());
  const void* tag_v_ptr = &values[0];

  ::moab::ErrorCode rval =
    iface->tag_set_data(
      tag.moabTag(), handles, tag_v_ptr);
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
::moab::Interface* extract_moab_interface( const smtk::mesh::InterfacePtr &iface)
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
smtk::mesh::ConnectivityStoragePtr Interface::connectivityStorage(
                                      const smtk::mesh::HandleRange& cells)
{
  //make boost shared_ptr
  smtk::mesh::ConnectivityStoragePtr cs(
                    new smtk::mesh::moab::ConnectivityStorage(this->m_iface.get(),
                                                              cells) );
  return cs;
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
//find all entity sets that have this exact name tag
smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle,
                                               const smtk::mesh::Neumann &neumann) const

{
  tag::QueryNeumannTag ntag(neumann.value(),this->moabInterface());

  smtk::mesh::HandleRange result;
  ::moab::ErrorCode rval;
  rval = m_iface->get_entities_by_type_and_tag(handle,
                                               ::moab::MBENTITYSET,
                                               ntag.moabTagPtr(),
                                               ntag.moabTagValuePtr(),
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
//get all cells held by this range handle of a given dimension
smtk::mesh::HandleRange Interface::getPoints(const smtk::mesh::HandleRange& cells) const

{
  smtk::mesh::HandleRange pointIds;
  m_iface->get_connectivity(cells, pointIds);
  return pointIds;
}

//----------------------------------------------------------------------------
bool Interface::getCoordinates(const smtk::mesh::HandleRange& points,
                               double* xyz) const

{
  if(points.empty() )
    {
    return false;
    }

  m_iface->get_coords(points,xyz);
  return true;
}

//----------------------------------------------------------------------------
bool Interface::getCoordinates(const smtk::mesh::HandleRange& points,
                               float* xyz) const

{
  if(points.empty())
    {
    return false;
    }
  //Efficiently re-use memory when fetching a significant number of points
  //this way we don't allocate a massive array of doubles, when that is most
  //likely going to trash the system memory, and defeat the users goal of keeping
  //memory usage low
  std::vector<double> coords;
  const std::size_t numPoints = points.size();
  const std::size_t numPointsPerLoop =524288;
  const std::size_t numLoops = numPoints / 524288;

  coords.reserve(numPointsPerLoop*3);
  smtk::mesh::HandleRange::const_iterator start = points.begin();


  std::size_t xyz_index = 0;
  for(std::size_t i=0; i < numLoops; ++i)
    {
    //determine where the end iterator should be
    smtk::mesh::HandleRange::const_iterator end = start + numPointsPerLoop;

    //needs to be insert so we use iterator insert, since we don't want
    //all points between start and end values, but only those that are
    //in the range. Think not 0 to N, but 0 - 10, 14 - N.
    ::moab::Range subset;
    subset.insert(start,end);

    //fetch all the coordinates for the start, end range
    m_iface->get_coords( subset,  &coords[0] );
    for(std::size_t i=0; i < (numPointsPerLoop*3); ++i)
      {
      xyz[xyz_index++] = static_cast<float>(coords[i]);
      }
    }

  std::size_t difference = numPoints - (numPointsPerLoop * numLoops);
  smtk::mesh::HandleRange::const_iterator end = start + difference;
  ::moab::Range subset;
  subset.insert(start,end);

  //fetch all the coordinates for the start, end range
  m_iface->get_coords( subset,  &coords[0] );

  for(std::size_t i=0; i < (difference*3); ++i)
    {
    xyz[xyz_index++] = static_cast<float>(coords[i]);
    }

  return true;
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
  return detail::computeDenseIntTagValues<smtk::mesh::Domain>(mtag,
                                                             meshsets,
                                                             this->moabInterface());
}


//----------------------------------------------------------------------------
std::vector< smtk::mesh::Dirichlet > Interface::computeDirichletValues(const smtk::mesh::HandleRange& meshsets) const
{
  tag::QueryDirichletTag dtag(0,this->moabInterface());
  return detail::computeDenseIntTagValues<smtk::mesh::Dirichlet>(dtag,
                                                              meshsets,
                                                              this->moabInterface());
}

//----------------------------------------------------------------------------
std::vector< smtk::mesh::Neumann > Interface::computeNeumannValues(const smtk::mesh::HandleRange& meshsets) const
{
  tag::QueryNeumannTag ntag(0,this->moabInterface());
  return detail::computeDenseIntTagValues<smtk::mesh::Neumann>(ntag,
                                                            meshsets,
                                                            this->moabInterface());
}

/**\brief Return the set of all UUIDs set on all entities in the meshsets.
  *
  */
smtk::common::UUIDArray Interface::computeModelEntities(const smtk::mesh::HandleRange& meshsets) const
{
  tag::QueryModelTag mtag(this->moabInterface());
  return detail::computeDenseOpaqueTagValues<smtk::common::UUIDArray>(
    mtag, meshsets, this->moabInterface());
}

//----------------------------------------------------------------------------
smtk::mesh::TypeSet Interface::computeTypes(const smtk::mesh::HandleRange& range) const
{
  typedef smtk::mesh::HandleRange::const_iterator cit;
  typedef ::smtk::mesh::CellType CellEnum;


  smtk::mesh::HandleRange meshes = range.subset_by_type( ::moab::MBENTITYSET );
  smtk::mesh::HandleRange cells = ::moab::subtract(range,meshes);

  smtk::mesh::CellTypes ctypes;

  //compute the type of the meshes, I don't want to try and extract the range
  //of cells for all the meshes, as that could be large
  for (cit i = meshes.begin(); i != meshes.end(); ++i)
    {
    const ::moab::EntityHandle& currentHandle = *i;
    for (std::size_t i = 0; i < ctypes.size(); ++i )
      {
      const CellEnum ce = static_cast<CellEnum>(i);
      const ::moab::EntityType moabEType =
      static_cast< ::moab::EntityType >(smtk::mesh::moab::smtkToMOABCell(ce));

      //some of the cell types that smtk supports moab doesn't support
      //so we can't query on those.
      int num = 0;
      m_iface->get_number_entities_by_type(currentHandle,
                                           static_cast< ::moab::EntityType >(moabEType),
                                           num,
                                           true);
      if(num > 0) { ctypes[ce] = true; }
      }
    }

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
  return smtk::mesh::TypeSet( ctypes, hasM, hasC );
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

  if(hasCells == false)
    {
    return false;
    }

  int skinDim = dimension - 1;

  //We need to first create the adjacencies from the requested dimension to the
  //dimension of the skin. The first step is create all the adjacencies from
  //the desired dimension to the skin dimension
  smtk::mesh::HandleRange allAdj;
  this->moabInterface()->get_adjacencies(cells, skinDim, true, allAdj);

  ::moab::Skinner skinner(this->moabInterface());
  ::moab::ErrorCode rval= skinner.find_skin(this->getRoot(),
                                            cells,
                                            skinDim,
                                            shell);


  if(rval != ::moab::MB_SUCCESS)
    { //if the skin extraction failed remove all cells we created
    this->moabInterface()->delete_entities(allAdj);
    }
  else
    { //remove any cell created by computing the adjacencies that isn't part
      //of the skin. This is done to keep the memory utilization low
    smtk::mesh::HandleRange unusedCells = ::moab::intersect(allAdj,shell);
    this->moabInterface()->delete_entities(unusedCells);
    }

  return (rval == ::moab::MB_SUCCESS);
 }

//----------------------------------------------------------------------------
bool Interface::mergeCoincidentContactPoints(const smtk::mesh::HandleRange& meshes,
                                            double tolerance) const
{
  //we want to merge the contact points for all dimensions
  //of the meshes, not just the highest dimension i expect
  smtk::mesh::moab::MergeMeshVertices meshmerger(this->moabInterface());
  ::moab::ErrorCode rval = meshmerger.merge_entities(meshes, tolerance);
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
bool Interface::setNeumann(const smtk::mesh::HandleRange& meshsets,
                           const smtk::mesh::Neumann& neumann) const
{
  if(meshsets.empty())
    {
    return true;
    }

  tag::QueryNeumannTag ntag(neumann.value(),this->moabInterface());

  //step 0 tag the meshsets
  bool tagged = detail::setDenseTagValues(ntag,meshsets,this->moabInterface());

  //step 1 find the highest dimension cells for the meshes.
  smtk::mesh::HandleRange cells;
  int dimension = 4;
  bool hasCells = false;
  while(hasCells == false && dimension >= 0)
    {
    --dimension;
    cells = this->getCells(meshsets, static_cast<smtk::mesh::DimensionType>(dimension));
    hasCells = !cells.empty();
    }

  //step 2 apply the neumann property to all cells for dimension that is 1 lower
  //since that would be the boundary dimension
  if(hasCells && dimension > 0)
    {
    --dimension;
    cells = this->getCells(meshsets, static_cast<smtk::mesh::DimensionType>(dimension));
    tagged = tagged && detail::setDenseTagValues(ntag,cells,this->moabInterface());
    }
  return tagged;
}

/**\brief Set the model entity assigned to each meshset member to \a ent.
  */
bool Interface::setModelEntity(
  const smtk::mesh::HandleRange& meshsets,
  const smtk::common::UUID& uuid) const
{
  if (meshsets.empty())
    return true;

  tag::QueryModelTag mtag(uuid, this->moabInterface());

  // I. Tag the meshsets
  bool tagged = detail::setDenseOpaqueTagValues(
    mtag, meshsets, this->moabInterface());

  // II. Tag the cells
  tagged &= detail::setDenseOpaqueTagValues(
    mtag, this->getCells(meshsets), this->moabInterface());
  return tagged;
}

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

  ::moab::Tag model_tag;
  m_iface->tag_get_handle(
    "MODEL", smtk::common::UUID::size(),
    ::moab::MB_TYPE_OPAQUE, model_tag,
    ::moab::MB_TAG_BYTES| ::moab::MB_TAG_CREAT| ::moab::MB_TAG_SPARSE);

  const void* tag_v_ptr = &modelUUID;

  ::moab::ErrorCode rval = m_iface->get_entities_by_type_and_tag(
    root, ::moab::MBENTITYSET, &model_tag, &tag_v_ptr, 1, result);
  (void)rval;
  return result;
}

//----------------------------------------------------------------------------
bool Interface::addAssociation(const smtk::common::UUID& modelUUID,
                               const smtk::mesh::HandleRange& range)
{
  if(range.empty() || !modelUUID)
    { //if empty range or invalid uuid
    return false;
    }

  // Set the value of the MODEL tag to the modelUUID for every item in the range
  ::moab::Tag model_tag;
  m_iface->tag_get_handle("MODEL",
                          smtk::common::UUID::size(), //this should be 128 bits
                          ::moab::MB_TYPE_OPAQUE,
                          model_tag,
                          ::moab::MB_TAG_BYTES| ::moab::MB_TAG_CREAT| ::moab::MB_TAG_SPARSE);

  const void* tag_v_ptr = &modelUUID;
  bool valid = true;
  typedef smtk::mesh::HandleRange::const_iterator cit;
  for(cit i=range.begin(); i!=range.end() && valid;++i)
    {
    const ::moab::EntityHandle& currentHandle = *i;
    ::moab::ErrorCode rval = m_iface->tag_set_data(model_tag,
                                                   &currentHandle,
                                                   1,
                                                   tag_v_ptr);
    valid = (rval == ::moab::MB_SUCCESS);
    }
  return valid;
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
void Interface::pointForEach(const HandleRange &points,
                             smtk::mesh::PointForEach& filter) const
{
  if(!points.empty())
    {
    //fetch a collection of points
    std::vector<double> coords;

    //determine the number of points. Break that into manageable chunks, we dont
    //want to allocate an array for hundreds of millions of points, instead
    //we would want to fetch a subset of points at a time.
    const std::size_t numPoints = points.size();

    const std::size_t numPointsPerLoop =524288;
    const std::size_t numLoops = numPoints / 524288;

    coords.reserve(numPointsPerLoop*3);
    smtk::mesh::HandleRange::const_iterator start = points.begin();


    for(std::size_t i=0; i < numLoops; ++i)
      {
      //determine where the end iterator should be
      smtk::mesh::HandleRange::const_iterator end = start + numPointsPerLoop;

      //needs to be insert so we use iterator insert, since we don't want
      //all points between start and end values, but only those that are
      //in the range. Think not 0 to N, but 0 - 10, 14 - N.
      ::moab::Range subset;
      subset.insert(start,end);

      //fetch all the coordinates for the start, end range
      m_iface->get_coords( subset,  &coords[0] );

      //call the filter for each point
      for(std::size_t offset = 0; start != end; offset+=3, ++start)
        {
        filter.forPoint( *start,
                          coords[offset],
                          coords[offset+1],
                          coords[offset+2] );
        }

      }

    std::size_t difference = numPoints - (numPointsPerLoop * numLoops);
    smtk::mesh::HandleRange::const_iterator end = start + difference;
    ::moab::Range subset;
    subset.insert(start,end);

    //fetch all the coordinates for the start, end range
    m_iface->get_coords( subset,  &coords[0] );

    //call the filter for each point
    for(std::size_t offset = 0; start != end; offset+=3, ++start)
      {
      filter.forPoint( *start,
                        coords[offset],
                        coords[offset+1],
                        coords[offset+2] );
      }
    }
  return;
}

//----------------------------------------------------------------------------
void Interface::cellForEach(smtk::mesh::PointConnectivity& pc,
                            smtk::mesh::CellForEach& filter) const
{
  if(!pc.is_empty())
    {
    smtk::mesh::CellType cellType;
    int size=0;
    const smtk::mesh::Handle* points;


    typedef smtk::mesh::HandleRange::const_iterator cit;

    if(filter.wantsCoordinates())
      {
      std::vector<double> coords;
      for(pc.initCellTraversal();
          pc.fetchNextCell(cellType, size, points) == true;
          )
        {
        coords.resize(size*3);

        //query to grab the coordinates for these points
        m_iface->get_coords(points,
                            size,
                            &coords[0]);
        //call the custom filter
        filter.pointIds(points);
        filter.coordinates(&coords);
        filter.forCell(cellType,size);
        }
      }
    else
      { //don't extract the coords
      for(pc.initCellTraversal();
          pc.fetchNextCell(cellType, size, points) == true;
          )
        {
        filter.pointIds(points);
        //call the custom filter
        filter.forCell(cellType,size);
        }
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
      filter.forMesh(singleMesh);
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
::moab::Interface* Interface::moabInterface() const
{
  return this->m_iface.get();
}

}
}
}
