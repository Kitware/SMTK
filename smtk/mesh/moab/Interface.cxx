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
#include "smtk/mesh/moab/BufferedCellAllocator.h"
#include "smtk/mesh/moab/ConnectivityStorage.h"
#include "smtk/mesh/moab/IncrementalAllocator.h"
#include "smtk/mesh/moab/MergeMeshVertices.h"
#include "smtk/mesh/moab/PointLocatorImpl.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/Core.hpp"
#include "moab/FileOptions.hpp"
#include "moab/Interface.hpp"

#include "moab/ReaderIface.hpp"
#include "moab/Skinner.hpp"
SMTK_THIRDPARTY_POST_INCLUDE


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
template<typename T, typename U>
T computeDenseOpaqueTagValue(
  U tag,
  const smtk::mesh::Handle& handle,
  ::moab::Interface* iface)
{
  std::vector<unsigned char> tag_values;
  tag_values.resize(tag.size());
  void* tag_v_ptr = &tag_values[0];

  // Fetch the tag for each item in the range in bulk
  ::moab::ErrorCode rval = iface->tag_get_data(tag.moabTag(), &handle, 1, tag_v_ptr);

  T result;
  if(rval == ::moab::MB_SUCCESS)
    {
    std::vector<unsigned char>::const_iterator i = tag_values.begin();
    result = T( &(*i), &(*i)+tag.size() );
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

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
template<typename T>
bool setDenseOpaqueTagValue(T tag, const smtk::mesh::Handle& handle,
                           ::moab::Interface* iface)
{
  //create a vector the same value so we can assign a tag
  std::vector<unsigned char> values;
  values.resize(tag.size());
  memcpy(&values[0], tag.value(), tag.size());
  const void* tag_v_ptr = &values[0];

  ::moab::ErrorCode rval =
    iface->tag_set_data(
      tag.moabTag(), &handle, 1, tag_v_ptr);
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
  m_alloc( ),
  m_bcAlloc( ),
  m_modified(false)
{
  this->m_alloc.reset( new smtk::mesh::moab::Allocator( this->m_iface.get() ) );
  this->m_bcAlloc.reset( new smtk::mesh::moab::BufferedCellAllocator( this->m_iface.get() ) );
  this->m_iAlloc.reset( new smtk::mesh::moab::IncrementalAllocator( this->m_iface.get() ) );
}

//----------------------------------------------------------------------------
Interface::~Interface()
{

}

//----------------------------------------------------------------------------
bool Interface::isModified() const
{
  return this->m_modified;
}

//----------------------------------------------------------------------------
smtk::mesh::AllocatorPtr Interface::allocator()
{
  //mark us as modified as the caller is going to add something to the database
  this->m_modified = true;
  return this->m_alloc;
}

//----------------------------------------------------------------------------
smtk::mesh::BufferedCellAllocatorPtr Interface::bufferedCellAllocator()
{
  //mark us as modified as the caller is going to add something to the database
  this->m_modified = true;
  return this->m_bcAlloc;
}

//----------------------------------------------------------------------------
smtk::mesh::IncrementalAllocatorPtr Interface::incrementalAllocator()
{
  //mark us as modified as the caller is going to add something to the database
  this->m_modified = true;
  static_cast<smtk::mesh::moab::IncrementalAllocator*>(this->m_iAlloc.get())->
    initialize();
  return this->m_iAlloc;
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
smtk::mesh::PointLocatorImplPtr Interface::pointLocator(
                                      const smtk::mesh::HandleRange& points)
{
  return smtk::mesh::PointLocatorImplPtr(
          new smtk::mesh::moab::PointLocatorImpl(this->m_iface.get(),
                                                 points) );
}

//----------------------------------------------------------------------------
smtk::mesh::PointLocatorImplPtr Interface::pointLocator(
                                      const double* const xyzs,
                                      std::size_t numPoints,
                                      bool ignoreZValues)
{
  if(numPoints == 0)
    {
    return smtk::mesh::PointLocatorImplPtr();
    }
  return smtk::mesh::PointLocatorImplPtr(
          new smtk::mesh::moab::PointLocatorImpl(this->m_iface.get(),
                                                 xyzs,
                                                 numPoints,
                                                 ignoreZValues) );
}

//----------------------------------------------------------------------------
smtk::mesh::PointLocatorImplPtr Interface::pointLocator(
                                      const float* const xyzs,
                                      std::size_t numPoints,
                                      bool ignoreZValues)
{
  if(numPoints == 0)
    {
    return smtk::mesh::PointLocatorImplPtr();
    }
  return smtk::mesh::PointLocatorImplPtr(
          new smtk::mesh::moab::PointLocatorImpl(this->m_iface.get(),
                                                 xyzs,
                                                 numPoints,
                                                 ignoreZValues) );
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
      hasDim = (cells.num_of_dimension(dimension) > 0);
      }

    //add the dim tag
    tag::QueryDimTag dimTag(dimension, this->moabInterface());
    m_iface->tag_set_data( dimTag.moabTag(),
                           &meshHandle, 1,
                           dimTag.moabTagValuePtr());
    }

  if(rval == ::moab::MB_SUCCESS)
    {
    this->m_modified = true;
    return true;
    }
  return false;
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
  for(int i = static_cast<int>((cellTypes.size() -1)); i >= 0; --i )
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
namespace { class GetCoords : public smtk::mesh::PointForEach {
public:
  std::size_t xyz_index;
  float* m_xyz;
  GetCoords(float* xyz): xyz_index(0), m_xyz(xyz) {}

  void forPoints(const smtk::mesh::HandleRange&,
                 std::vector<double>& xyz,
                 bool&)
  {
    //use local variable instead of member to help locality
    std::size_t index = xyz_index;
    for(std::vector<double>::const_iterator i=xyz.begin(); i!=xyz.end(); ++i)
      {
      this->m_xyz[index++] = static_cast<float>(*i);
      }
    this->xyz_index = index;
  }
}; }

//----------------------------------------------------------------------------
bool Interface::getCoordinates(const smtk::mesh::HandleRange& points,
                               float* xyz) const

{
  if(points.empty())
    {
    return false;
    }

  //requires that for_each is serial
  GetCoords functor(xyz);
  this->pointForEach(points, functor);
  return true;
}

//----------------------------------------------------------------------------
bool Interface::setCoordinates(const smtk::mesh::HandleRange& points,
                               const double* const xyz)

{
  if(points.empty())
    {
    return false;
    }

  m_iface->set_coords(points,xyz);
  return true;
}

//----------------------------------------------------------------------------
namespace { class SetCoords : public smtk::mesh::PointForEach {
public:
  std::size_t xyz_index;
  const float* const m_xyz;
  SetCoords(const float* const xyz): xyz_index(0), m_xyz(xyz) {}

  void forPoints(const smtk::mesh::HandleRange&,
                 std::vector<double>& xyz,
                 bool& coordinatesModified)
  {
    coordinatesModified = true;
    //use local variable instead of member to help locality
    std::size_t index = this->xyz_index;
    for(std::vector<double>::iterator i=xyz.begin(); i!=xyz.end(); ++i)
      {
      *i = static_cast<double>(this->m_xyz[index++]);
      }
    this->xyz_index = index;
  }
}; }

//----------------------------------------------------------------------------
bool Interface::setCoordinates(const smtk::mesh::HandleRange& points,
                               const float* const xyz)

{
  //requires that for_each is serial
  SetCoords functor(xyz);
  this->pointForEach(points, functor);
  return false;
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

//----------------------------------------------------------------------------
/**\brief Return the set of all UUIDs set on all entities in the meshsets.
  *
  */
smtk::common::UUIDArray Interface::computeModelEntities(const smtk::mesh::HandleRange& meshsets) const
{
  tag::QueryEntRefTag mtag(this->moabInterface());
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
  for (cit m = meshes.begin(); m != meshes.end(); ++m)
    {
    const ::moab::EntityHandle& currentHandle = *m;
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
                                             double tolerance)
{
  if(meshes.empty())
    {
    //I can't see a reason why we should consider a merge of nothing to be a
    //failure. So we return true.
    return true;
    }

  //we want to merge the contact points for all dimensions
  //of the meshes, not just the highest dimension i expect
  smtk::mesh::moab::MergeMeshVertices meshmerger(this->moabInterface());
  ::moab::ErrorCode rval = meshmerger.merge_entities(meshes, tolerance);
  if(rval == ::moab::MB_SUCCESS)
    {
    this->m_modified = true;
    return true;
    }
  return false;
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
  bool tagged = detail::setDenseTagValues(mtag,meshsets,this->moabInterface());
  if(tagged)
    {
    this->m_modified = true;
    }
  return tagged;
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

  const bool tagged = cellsTagged && meshesTagged;
  if(tagged)
    {
    this->m_modified = true;
    }
  return tagged;
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

  if(tagged)
    {
    this->m_modified = true;
    }
  return tagged;
}

//----------------------------------------------------------------------------
/**\brief Set the model entity assigned to each meshset member to \a ent.
  */
bool Interface::setAssociation(const smtk::common::UUID& modelUUID,
                               const smtk::mesh::HandleRange& range) const
{
  if(range.empty() || !modelUUID)
    { //if empty range or invalid uuid
    return false;
    }

  tag::QueryEntRefTag mtag(modelUUID, this->moabInterface());

  //Tag the meshsets
  bool tagged = detail::setDenseOpaqueTagValues(mtag,
                                                range,
                                                this->moabInterface());
  if(tagged)
    {
    this->m_modified = true;
    }
  return tagged;
}

//----------------------------------------------------------------------------
/**\brief Find mesh entities associated with the given model entity.
  *
  */
smtk::mesh::HandleRange Interface::findAssociations(
  const smtk::mesh::Handle& root,
  const smtk::common::UUID& modelUUID) const
{
  smtk::mesh::HandleRange result;
  if (!modelUUID)
    {
    return result;
    }

  tag::QueryEntRefTag mtag(modelUUID, this->moabInterface());

  ::moab::ErrorCode rval;
  rval = m_iface->get_entities_by_type_and_tag(root,
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
// brief Set the model entity assigned to the root of this interface.
//
bool Interface::setRootAssociation(const smtk::common::UUID& modelUUID) const
{
  if (!modelUUID)
    {
    return false;
    }

  smtk::mesh::Handle root = m_iface->get_root_set();
  tag::QueryRootModelEntTag mtag(modelUUID, this->moabInterface());

  //Tag the root
  bool tagged = detail::setDenseOpaqueTagValue(mtag,
                                               root,
                                               this->moabInterface());
  if(tagged)
    {
    this->m_modified = true;
    }
  return tagged;
}

//----------------------------------------------------------------------------
/// brief Get the model entity assigned to the root of this interface.
//
smtk::common::UUID Interface::rootAssociation() const
{
  //first we need to verify that we have a ROOT_MODEL tag first
  std::vector< ::moab::Tag > tag_handles;
  smtk::mesh::Handle root = m_iface->get_root_set();
  m_iface->tag_get_tags_on_entity(root,tag_handles);

  if(tag_handles.size() > 0)
    {
    tag::QueryRootModelEntTag mtag(this->moabInterface());
    return detail::computeDenseOpaqueTagValue<smtk::common::UUID>(mtag,
                                                                  root,
                                                                  this->moabInterface());
    }
  return smtk::common::UUID::null();
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

    const std::size_t numPointsPerLoop=65536; //selected so that buffer is ~1MB
    const std::size_t numLoops = numPoints/65536;

    //We explicitly reserve than resize to avoid the cost of resize behavior
    //of setting the value of each element to T(). But at the same time we
    //need to use resize to set the proper size of the vector ( reserve == capacity )
    coords.reserve(numPointsPerLoop*3);
    coords.resize(numPointsPerLoop*3);
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

      //call the filter for this chunk of points
      bool shouldBeSaved = false;
      filter.forPoints(subset, coords, shouldBeSaved);
      if(shouldBeSaved)
        {
        //save t
        m_iface->set_coords( subset, &coords[0] );
        }
      start += numPointsPerLoop;

      }

    std::size_t difference = numPoints - (numPointsPerLoop * numLoops);

    //Update the size of the coords, will not cause a re-alloc since
    //the capacity of coords > difference*3
    coords.resize(difference*3);

    smtk::mesh::HandleRange::const_iterator end = start + difference;
    ::moab::Range subset;
    subset.insert(start,end);

    //fetch all the coordinates for the start, end range
    m_iface->get_coords( subset,  &coords[0] );

    //call the filter for the rest of the points
    bool shouldBeSaved = false;
    filter.forPoints(subset, coords, shouldBeSaved);
    if(shouldBeSaved)
      {
      //save t
      m_iface->set_coords( subset, &coords[0] );
      }
    }
  return;
}

//----------------------------------------------------------------------------
void Interface::cellForEach(const HandleRange &cells,
                            smtk::mesh::PointConnectivity& pc,
                            smtk::mesh::CellForEach& filter) const
{
  if(!pc.is_empty())
    {
    smtk::mesh::CellType cellType;
    int size=0;
    const smtk::mesh::Handle* points;


    smtk::mesh::HandleRange::const_iterator currentCell = cells.begin();
    if(filter.wantsCoordinates())
      {
      std::vector<double> coords;
      for(pc.initCellTraversal();
          pc.fetchNextCell(cellType, size, points) == true;
          currentCell++
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
        filter.forCell(*currentCell,cellType,size);
        }
      }
    else
      { //don't extract the coords
      for(pc.initCellTraversal();
          pc.fetchNextCell(cellType, size, points) == true;
          currentCell++
          )
        {
        filter.pointIds(points);
        //call the custom filter
        filter.forCell(*currentCell, cellType,size);
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
  bool isDeleted = false;
  if(toDel.all_of_type(::moab::MBENTITYSET))
    {
    //first remove any model entity relation-ship these meshes have
    tag::QueryEntRefTag mtag(this->moabInterface());
    m_iface->tag_delete_data(mtag.moabTag(), toDel);

    //we are all moab entity sets, fine to delete
    const::moab::ErrorCode rval = m_iface->delete_entities(toDel);
    isDeleted = (rval == ::moab::MB_SUCCESS);
    }
  else if(toDel.num_of_type(::moab::MBENTITYSET) == 0)
    {
    //first remove any model entity relation-ship these cells have
    tag::QueryEntRefTag mtag(this->moabInterface());
    m_iface->tag_delete_data(mtag.moabTag(), toDel);

    //for now we are going to avoid deleting any vertex
    smtk::mesh::HandleRange vertCells = toDel.subset_by_dimension(0);
    smtk::mesh::HandleRange otherCells = ::moab::subtract(toDel, vertCells);

    //we have zero entity sets so we must be all cells/coords
    const ::moab::ErrorCode rval = m_iface->delete_entities(otherCells);

    //we don't delete the vertices, as those can't be explicitly deleted
    //instead they are deleted when the mesh goes away
    return (rval == ::moab::MB_SUCCESS);
    }
  if(isDeleted)
    {
    this->m_modified = true;
    }
  return isDeleted;

}

//----------------------------------------------------------------------------
::moab::Interface* Interface::moabInterface() const
{
  return this->m_iface.get();
}


}
}
}
