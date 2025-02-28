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

#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/QueryTypes.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/moab/Allocator.h"
#include "smtk/mesh/moab/BufferedCellAllocator.h"
#include "smtk/mesh/moab/CellTypeToType.h"
#include "smtk/mesh/moab/ClosestPoint.h"
#include "smtk/mesh/moab/ConnectivityStorage.h"
#include "smtk/mesh/moab/DistanceTo.h"
#include "smtk/mesh/moab/HandleRangeToRange.h"
#include "smtk/mesh/moab/IncrementalAllocator.h"
#include "smtk/mesh/moab/MergeMeshVertices.h"
#include "smtk/mesh/moab/PointLocatorImpl.h"
#include "smtk/mesh/moab/RandomPoint.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/Core.hpp"
#include "moab/FileOptions.hpp"
#include "moab/Interface.hpp"

#include "moab/ErrorHandler.hpp"
#include "moab/ReaderIface.hpp"
#include "moab/Skinner.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#define BEING_INCLUDED_BY_INTERFACE_CXX
//required to go after moab includes
#include "smtk/mesh/moab/Tags.h"
#undef BEING_USED_BY_INTERFACE_CXX

#include <algorithm>
#include <cstring>
#include <memory>
#include <set>

namespace smtk
{
namespace mesh
{
namespace moab
{

namespace detail
{
smtk::mesh::HandleRange vectorToHandleRange(std::vector<::moab::EntityHandle>& vresult)
{
  smtk::mesh::HandleRange resulting_range;
  const std::size_t size = vresult.size();
  for (std::size_t i = 0; i < size;)
  {
    std::size_t j;
    for (j = i + 1; j < size && vresult[j] == 1 + vresult[j - 1]; j++)
      ;
    //empty for loop
    resulting_range.insert(
      resulting_range.end(), smtk::mesh::HandleInterval(vresult[i], vresult[i] + (j - i - 1)));
    i = j;
  }
  return resulting_range;
}

template<typename T, typename U>
std::vector<T>
computeDenseIntTagValues(U tag, const ::moab::Range& meshsets, ::moab::Interface* iface)
{
  std::vector<T> result;

  //fetch all entities with the given tag
  ::moab::Range entitiesWithTag;
  iface->get_entities_by_type_and_tag(
    iface->get_root_set(), ::moab::MBENTITYSET, tag.moabTagPtr(), nullptr, 1, entitiesWithTag);

  //we have all entity sets that have the this tag
  //now we need to find the subset that is part of our
  //HandleRange
  entitiesWithTag = ::moab::intersect(entitiesWithTag, meshsets);

  //return early if nothing has the tag.
  //this also makes it safer to derefence the std vector below
  if (entitiesWithTag.empty())
  {
    return result;
  }

  //allocate a vector large enough to hold the tag values for every element
  std::vector<int> tag_values;
  tag_values.resize(entitiesWithTag.size());
  void* tag_v_ptr = tag_values.data();

  //fetch the tag for each item in the range in bulk
  iface->tag_get_data(tag.moabTag(), entitiesWithTag, tag_v_ptr);

  //find and remove duplicates
  std::sort(tag_values.begin(), tag_values.end());
  tag_values.erase(std::unique(tag_values.begin(), tag_values.end()), tag_values.end());

  //for each tag value convert it to a type T, where T is an
  //IntTag from smtk::mesh
  result.reserve(tag_values.size());
  typedef std::vector<int>::const_iterator cit;
  for (cit i = tag_values.begin(); i != tag_values.end(); ++i)
  {
    result.push_back(T(*i));
  }
  return result;
}

template<typename T, typename U>
T computeDenseOpaqueTagValues(U tag, const ::moab::Range& meshsets, ::moab::Interface* iface)
{
  T result;

  // Fetch all entities with the given tag
  ::moab::Range entitiesWithTag;
  iface->get_entities_by_type_and_tag(
    iface->get_root_set(), ::moab::MBENTITYSET, tag.moabTagPtr(), nullptr, 1, entitiesWithTag);

  // We have all entity sets that have the this tag; now we need
  // to find the subset that is part of our HandleRange.
  entitiesWithTag = ::moab::intersect(entitiesWithTag, meshsets);

  // Return early if nothing has the tag.
  // This also makes it safer to derefence the std vector below
  if (entitiesWithTag.empty())
    return result;

  std::vector<unsigned char> tag_values;
  tag_values.resize(entitiesWithTag.size() * tag.size());
  void* tag_v_ptr = tag_values.data();

  // Fetch the tag for each item in the range in bulk
  iface->tag_get_data(tag.moabTag(), entitiesWithTag, tag_v_ptr);

  // For each tag value convert it to a value type for the
  // output container T.
  typedef std::vector<unsigned char>::const_iterator cit;
  for (cit i = tag_values.begin(); i != tag_values.end(); i += tag.size())
  {
    result.push_back(typename T::value_type(&(*i), &(*i) + tag.size()));
  }
  return result;
}

template<typename T, typename U>
T computeDenseOpaqueTagValue(U tag, const smtk::mesh::Handle& handle, ::moab::Interface* iface)
{
  std::vector<unsigned char> tag_values;
  tag_values.resize(tag.size());
  void* tag_v_ptr = tag_values.data();

  // Fetch the tag for each item in the range in bulk
  ::moab::ErrorCode rval = iface->tag_get_data(tag.moabTag(), &handle, 1, tag_v_ptr);

  T result;
  if (rval == ::moab::MB_SUCCESS)
  {
    std::vector<unsigned char>::const_iterator i = tag_values.begin();
    result = T(&(*i), &(*i) + tag.size());
  }
  return result;
}

template<typename T>
bool setDenseTagValues(T tag, const ::moab::Range& handles, ::moab::Interface* iface)
{
  //create a vector the same value so we can assign a tag
  std::vector<int> values;
  values.resize(handles.size(), tag.value());
  const void* tag_v_ptr = values.data();

  ::moab::ErrorCode rval = iface->tag_set_data(tag.moabTag(), handles, tag_v_ptr);
  return (rval == ::moab::MB_SUCCESS);
}

template<typename T>
bool setDenseOpaqueTagValues(T tag, const ::moab::Range& handles, ::moab::Interface* iface)
{
  //create a vector the same value so we can assign a tag
  std::vector<unsigned char> values;
  values.resize(handles.size() * tag.size());
  for (std::size_t i = 0; i < handles.size(); ++i)
    memcpy(&values[i * tag.size()], tag.value(), tag.size());
  const void* tag_v_ptr = values.data();

  ::moab::ErrorCode rval = iface->tag_set_data(tag.moabTag(), handles, tag_v_ptr);
  return (rval == ::moab::MB_SUCCESS);
}

template<typename T>
bool setDenseOpaqueTagValue(T tag, const smtk::mesh::Handle& handle, ::moab::Interface* iface)
{
  //create a vector the same value so we can assign a tag
  std::vector<unsigned char> values;
  values.resize(tag.size());
  memcpy(values.data(), tag.value(), tag.size());
  const void* tag_v_ptr = values.data();

  ::moab::ErrorCode rval = iface->tag_set_data(tag.moabTag(), &handle, 1, tag_v_ptr);
  return (rval == ::moab::MB_SUCCESS);
}

} // namespace detail

//construct an empty interface instance
smtk::mesh::moab::InterfacePtr make_interface()
{
  //Core is a fully implemented moab::Interface
  return std::make_shared<smtk::mesh::moab::Interface>();
}

//Given a smtk::mesh Interface convert it to a smtk::mesh::moab interface
smtk::mesh::moab::InterfacePtr extract_interface(const smtk::mesh::ResourcePtr& c)
{
  return smtk::dynamic_pointer_cast<smtk::mesh::moab::Interface>(c->interface());
}

//Given a smtk::mesh Interface convert it to a smtk::mesh::moab interface, and than
//extract the raw moab interface pointer from that
::moab::Interface* extract_moab_interface(const smtk::mesh::InterfacePtr& iface)
{
  smtk::mesh::moab::Interface* mi = dynamic_cast<smtk::mesh::moab::Interface*>(iface.get());
  return (mi == nullptr) ? nullptr : mi->moabInterface();
}

Interface::Interface()
  : m_iface(new ::moab::Core())
{
  m_alloc = std::make_shared<smtk::mesh::moab::Allocator>(m_iface.get());
  m_bcAlloc = std::make_shared<smtk::mesh::moab::BufferedCellAllocator>(m_iface.get());
  m_iAlloc = std::make_shared<smtk::mesh::moab::IncrementalAllocator>(m_iface.get());

  // Moab has become increasingly verbose. For now, let's make it quiet.
  //
  // TODO: pipe moab messaging through SMTK's messaging (perhaps with some
  //       filtering)
  ::moab::MBErrorHandler_Finalize();
}

Interface::~Interface() = default;

bool Interface::isModified() const
{
  return m_modified;
}

smtk::mesh::AllocatorPtr Interface::allocator()
{
  //mark us as modified as the caller is going to add something to the database
  m_modified = true;
  return m_alloc;
}

smtk::mesh::BufferedCellAllocatorPtr Interface::bufferedCellAllocator()
{
  //mark us as modified as the caller is going to add something to the database
  m_modified = true;
  std::static_pointer_cast<smtk::mesh::moab::BufferedCellAllocator>(m_bcAlloc)->clear();
  return m_bcAlloc;
}

smtk::mesh::IncrementalAllocatorPtr Interface::incrementalAllocator()
{
  //mark us as modified as the caller is going to add something to the database
  m_modified = true;
  static_cast<smtk::mesh::moab::IncrementalAllocator*>(m_iAlloc.get())->initialize();
  return m_iAlloc;
}

smtk::mesh::ConnectivityStoragePtr Interface::connectivityStorage(
  const smtk::mesh::HandleRange& cells)
{
  //make boost shared_ptr
  smtk::mesh::ConnectivityStoragePtr cs(
    new smtk::mesh::moab::ConnectivityStorage(m_iface.get(), cells));
  return cs;
}

smtk::mesh::PointLocatorImplPtr Interface::pointLocator(const smtk::mesh::HandleRange& points)
{
  return smtk::mesh::PointLocatorImplPtr(
    new smtk::mesh::moab::PointLocatorImpl(m_iface.get(), smtkToMOABRange(points)));
}

smtk::mesh::PointLocatorImplPtr Interface::pointLocator(
  std::size_t numPoints,
  const std::function<std::array<double, 3>(std::size_t)>& coordinates)
{
  if (numPoints == 0)
  {
    return smtk::mesh::PointLocatorImplPtr();
  }
  return smtk::mesh::PointLocatorImplPtr(
    new smtk::mesh::moab::PointLocatorImpl(m_iface.get(), numPoints, coordinates));
}

smtk::mesh::Handle Interface::getRoot() const
{
  return m_iface->get_root_set();
}

void Interface::registerQueries(smtk::mesh::Resource& resource) const
{
  resource.queries().registerQuery<smtk::mesh::moab::ClosestPoint>();
  resource.queries().registerQuery<smtk::mesh::moab::DistanceTo>();
  resource.queries().registerQuery<smtk::mesh::moab::RandomPoint>();
}

bool Interface::createMesh(const smtk::mesh::HandleRange& cells, smtk::mesh::Handle& meshHandle)
{
  if (cells.empty())
  {
    return false;
  }

  ::moab::Range moabCells = smtkToMOABRange(cells);

  //make sure the cells are actually cells instead of meshsets.
  //we currently don't want this allow adding sub meshsets
  if (moabCells.num_of_type(::moab::MBENTITYSET) != 0)
  {
    return false;
  }

  const unsigned int options = 0;
  ::moab::ErrorCode rval = m_iface->create_meshset(options, meshHandle);
  if (rval == ::moab::MB_SUCCESS)
  {
    m_iface->add_entities(meshHandle, moabCells);
    m_iface->add_parent_child(m_iface->get_root_set(), meshHandle);

    int dimension = 4;
    bool hasDim = false;
    while (!hasDim && dimension >= 0)
    {
      //by starting at 4 and decrementing we don't need to branch
      //on hasDim to see if we need to decrement at the end of
      //the while loop
      --dimension;

      //iterate the entities and find the higest dimension of cell.
      //once that is found add a geom sparse tag to the mesh
      hasDim = (moabCells.num_of_dimension(dimension) > 0);
    }

    //add the dim tag
    tag::QueryDimTag dimTag(dimension, this->moabInterface());
    m_iface->tag_set_data(dimTag.moabTag(), &meshHandle, 1, dimTag.moabTagValuePtr());
  }

  if (rval == ::moab::MB_SUCCESS)
  {
    m_modified = true;
    return true;
  }
  return false;
}

std::size_t Interface::numMeshes(smtk::mesh::Handle handle) const
{
  int num_ents = 0;
  m_iface->get_number_entities_by_type(handle, ::moab::MBENTITYSET, num_ents);
  return static_cast<std::size_t>(num_ents);
}

smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle) const

{
  ::moab::Range range;
  m_iface->get_entities_by_type(handle, ::moab::MBENTITYSET, range);
  return moabToSMTKRange(range);
}

smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle, int dimension) const

{
  typedef std::vector<::moab::EntityHandle>::const_iterator it;

  //use a vector since we are going to do single element iteration, and
  //removal.
  std::vector<::moab::EntityHandle> all_ents;
  std::vector<::moab::EntityHandle> matching_ents;
  m_iface->get_entities_by_type(handle, ::moab::MBENTITYSET, all_ents);

  //add all meshsets that have at least a single cell of the given dimension
  for (it i = all_ents.begin(); i != all_ents.end(); ++i)
  {
    ::moab::Range cells_of_given_dim;
    m_iface->get_entities_by_dimension(*i, dimension, cells_of_given_dim);
    if (!cells_of_given_dim.empty())
    {
      matching_ents.push_back(*i);
    }
  }
  return detail::vectorToHandleRange(matching_ents);
}

//find all entity sets that have this exact name tag
smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle, const std::string& name)
  const

{
  typedef std::vector<::moab::EntityHandle>::const_iterator it;

  //use a vector since we are going to do single element iteration, and
  //removal.
  std::vector<::moab::EntityHandle> all_ents;
  std::vector<::moab::EntityHandle> matching_ents;
  m_iface->get_entities_by_type(handle, ::moab::MBENTITYSET, all_ents);

  //see which ones have a a matching name, and if so add it
  //we don't use get_entities_by_type_and_tag as it doesn't
  //seem to work with name tags
  tag::QueryNameTag query_name(this->moabInterface());
  for (it i = all_ents.begin(); i != all_ents.end(); ++i)
  {
    const bool has_name = query_name.fetch_name(*i);
    if (has_name && (name == query_name.current_name()))
    { //has a matching name
      matching_ents.push_back(*i);
    }
  }
  return detail::vectorToHandleRange(matching_ents);
}

//find all entity sets that have this exact name tag
smtk::mesh::HandleRange Interface::getMeshsets(
  smtk::mesh::Handle handle,
  const smtk::mesh::Domain& domain) const

{
  tag::QueryMaterialTag mtag(domain.value(), this->moabInterface());

  ::moab::Range result;
  ::moab::ErrorCode rval;
  rval = m_iface->get_entities_by_type_and_tag(
    handle, ::moab::MBENTITYSET, mtag.moabTagPtr(), mtag.moabTagValuePtr(), 1, result);
  if (rval != ::moab::MB_SUCCESS)
  {
    result.clear();
  }
  return moabToSMTKRange(result);
}

//find all entity sets that have this exact name tag
smtk::mesh::HandleRange Interface::getMeshsets(
  smtk::mesh::Handle handle,
  const smtk::mesh::Dirichlet& dirichlet) const

{
  tag::QueryDirichletTag dtag(dirichlet.value(), this->moabInterface());

  ::moab::Range result;
  ::moab::ErrorCode rval;
  rval = m_iface->get_entities_by_type_and_tag(
    handle, ::moab::MBENTITYSET, dtag.moabTagPtr(), dtag.moabTagValuePtr(), 1, result);
  if (rval != ::moab::MB_SUCCESS)
  {
    result.clear();
  }
  return moabToSMTKRange(result);
}

//find all entity sets that have this exact name tag
smtk::mesh::HandleRange Interface::getMeshsets(
  smtk::mesh::Handle handle,
  const smtk::mesh::Neumann& neumann) const

{
  tag::QueryNeumannTag ntag(neumann.value(), this->moabInterface());

  ::moab::Range result;
  ::moab::ErrorCode rval;
  rval = m_iface->get_entities_by_type_and_tag(
    handle, ::moab::MBENTITYSET, ntag.moabTagPtr(), ntag.moabTagValuePtr(), 1, result);
  if (rval != ::moab::MB_SUCCESS)
  {
    result.clear();
  }
  return moabToSMTKRange(result);
}

//get all cells held by this range
smtk::mesh::HandleRange Interface::getCells(const smtk::mesh::HandleRange& meshsets) const

{
  // get all non-meshset entities in meshset, including in contained meshsets
  ::moab::Range entitiesCells;
  for (auto i = boost::icl::elements_begin(meshsets); i != boost::icl::elements_end(meshsets); ++i)
  {
    //get_entities_by_handle appends to the range given
    m_iface->get_entities_by_handle(*i, entitiesCells, true);
  }
  return moabToSMTKRange(entitiesCells);
}

//get all cells held by this range handle of a given cell type
smtk::mesh::HandleRange Interface::getCells(
  const smtk::mesh::HandleRange& meshsets,
  smtk::mesh::CellType cellType) const
{
  int moabCellType = smtk::mesh::moab::smtkToMOABCell(cellType);

  ::moab::Range entitiesCells;

  // get all non-meshset entities in meshset of a given cell type
  for (auto i = boost::icl::elements_begin(meshsets); i != boost::icl::elements_end(meshsets); ++i)
  {
    //get_entities_by_type appends to the range given
    m_iface->get_entities_by_type(
      *i, static_cast<::moab::EntityType>(moabCellType), entitiesCells, true);
  }

  return moabToSMTKRange(entitiesCells);
}

//get all cells held by this range handle of a given cell type(s)
smtk::mesh::HandleRange Interface::getCells(
  const smtk::mesh::HandleRange& meshsets,
  const smtk::mesh::CellTypes& cellTypes) const

{
  const std::size_t cellTypesToFind = cellTypes.count();
  if (cellTypesToFind == cellTypes.size())
  { //if all the cellTypes are enabled we should just use get_cells
    //all() method can't be used as it was added in C++11
    return this->getCells(meshsets);
  }
  else if (cellTypesToFind == 0)
  {
    return smtk::mesh::HandleRange();
  }

  //we now search from highest cell type to lowest cell type adding everything
  //to the range. The reason for this is that ranges perform best when inserting
  //from high to low values
  smtk::mesh::HandleRange entitiesCells;
  for (int i = static_cast<int>((cellTypes.size() - 1)); i >= 0; --i)
  {
    //skip all cell types we don't have
    if (!cellTypes[i])
    {
      continue;
    }

    smtk::mesh::CellType currentCellType = static_cast<smtk::mesh::CellType>(i);

    smtk::mesh::HandleRange cellEnts = this->getCells(meshsets, currentCellType);

    entitiesCells += cellEnts;
  }

  return entitiesCells;
}

//get all cells held by this range handle of a given dimension
smtk::mesh::HandleRange Interface::getCells(
  const smtk::mesh::HandleRange& meshsets,
  smtk::mesh::DimensionType dim) const

{
  const int dimension = static_cast<int>(dim);

  //get all non-meshset entities of a given dimension
  ::moab::Range entitiesCells;
  for (auto i = boost::icl::elements_begin(meshsets); i != boost::icl::elements_end(meshsets); ++i)
  {
    //get_entities_by_dimension appends to the range given
    m_iface->get_entities_by_dimension(*i, dimension, entitiesCells, true);
  }
  return moabToSMTKRange(entitiesCells);
}

//get all cells held by this range handle of a given dimension
smtk::mesh::HandleRange Interface::getPoints(
  const smtk::mesh::HandleRange& cells,
  bool boundary_only) const

{
  ::moab::Range moabCells = smtkToMOABRange(cells);
  ::moab::Range pointIds;
  m_iface->get_connectivity(moabCells, pointIds, boundary_only);
  return moabToSMTKRange(pointIds);
}

bool Interface::getCoordinates(const smtk::mesh::HandleRange& points, double* xyz) const

{
  if (points.empty())
  {
    return false;
  }

  m_iface->get_coords(smtkToMOABRange(points), xyz);
  return true;
}

namespace
{
class GetCoords : public smtk::mesh::PointForEach
{
public:
  std::size_t xyz_index{ 0 };
  float* m_xyz;
  GetCoords(float* xyz)
    : m_xyz(xyz)
  {
  }

  void forPoints(
    const smtk::mesh::HandleRange& /*pointIds*/,
    std::vector<double>& xyz,
    bool& /*coordinatesModified*/) override
  {
    //use local variable instead of member to help locality
    std::size_t index = xyz_index;
    for (std::vector<double>::const_iterator i = xyz.begin(); i != xyz.end(); ++i)
    {
      m_xyz[index++] = static_cast<float>(*i);
    }
    this->xyz_index = index;
  }
};
} // namespace

bool Interface::getCoordinates(const smtk::mesh::HandleRange& points, float* xyz) const

{
  if (points.empty())
  {
    return false;
  }

  //requires that for_each is serial
  GetCoords functor(xyz);
  this->pointForEach(points, functor);
  return true;
}

bool Interface::setCoordinates(const smtk::mesh::HandleRange& points, const double* const xyz)

{
  if (points.empty())
  {
    return false;
  }

  m_iface->set_coords(smtkToMOABRange(points), xyz);
  return true;
}

namespace
{
class SetCoords : public smtk::mesh::PointForEach
{
public:
  std::size_t xyz_index{ 0 };
  const float* const m_xyz;
  SetCoords(const float* const xyz)
    : m_xyz(xyz)
  {
  }

  void forPoints(
    const smtk::mesh::HandleRange& /*pointIds*/,
    std::vector<double>& xyz,
    bool& coordinatesModified) override
  {
    coordinatesModified = true;
    //use local variable instead of member to help locality
    std::size_t index = this->xyz_index;
    for (std::vector<double>::iterator i = xyz.begin(); i != xyz.end(); ++i)
    {
      *i = static_cast<double>(m_xyz[index++]);
    }
    this->xyz_index = index;
  }
};
} // namespace

bool Interface::setCoordinates(const smtk::mesh::HandleRange& points, const float* const xyz)

{
  //requires that for_each is serial
  SetCoords functor(xyz);
  this->pointForEach(points, functor);
  return false;
}

std::string Interface::name(const smtk::mesh::Handle& meshset) const
{
  //construct a name tag query helper class
  tag::QueryNameTag query_name(this->moabInterface());

  if (query_name.fetch_name(meshset))
  {
    return std::string(query_name.current_name());
  }

  return std::string();
}

bool Interface::setName(const smtk::mesh::Handle& meshset, const std::string& name)
{
  //construct a name tag query helper class
  tag::QueryNameTag query_name(this->moabInterface());

  return query_name.set_name(meshset, name);
}

std::vector<std::string> Interface::computeNames(const smtk::mesh::HandleRange& meshsets) const
{
  //construct a name tag query helper class
  tag::QueryNameTag query_name(this->moabInterface());

  std::set<std::string> unique_names;
  for (auto i = boost::icl::elements_begin(meshsets); i != boost::icl::elements_end(meshsets); ++i)
  {
    const bool has_name = query_name.fetch_name(*i);
    if (has_name)
    {
      unique_names.insert(std::string(query_name.current_name()));
    }
  }
  //return a vector of the unique names
  return std::vector<std::string>(unique_names.begin(), unique_names.end());
}

std::vector<smtk::mesh::Domain> Interface::computeDomainValues(
  const smtk::mesh::HandleRange& meshsets) const
{
  tag::QueryMaterialTag mtag(0, this->moabInterface());
  return detail::computeDenseIntTagValues<smtk::mesh::Domain>(
    mtag, smtkToMOABRange(meshsets), this->moabInterface());
}

std::vector<smtk::mesh::Dirichlet> Interface::computeDirichletValues(
  const smtk::mesh::HandleRange& meshsets) const
{
  tag::QueryDirichletTag dtag(0, this->moabInterface());
  return detail::computeDenseIntTagValues<smtk::mesh::Dirichlet>(
    dtag, smtkToMOABRange(meshsets), this->moabInterface());
}

std::vector<smtk::mesh::Neumann> Interface::computeNeumannValues(
  const smtk::mesh::HandleRange& meshsets) const
{
  tag::QueryNeumannTag ntag(0, this->moabInterface());
  return detail::computeDenseIntTagValues<smtk::mesh::Neumann>(
    ntag, smtkToMOABRange(meshsets), this->moabInterface());
}

/**\brief Return the set of all UUIDs set on all entities in the meshsets.
  *
  */
smtk::common::UUIDArray Interface::computeModelEntities(
  const smtk::mesh::HandleRange& meshsets) const
{
  tag::QueryEntRefTag mtag(this->moabInterface());
  return detail::computeDenseOpaqueTagValues<smtk::common::UUIDArray>(
    mtag, smtkToMOABRange(meshsets), this->moabInterface());
}

smtk::mesh::TypeSet Interface::computeTypes(const smtk::mesh::HandleRange& range) const
{
  typedef ::moab::Range::const_iterator cit;
  typedef ::smtk::mesh::CellType CellEnum;

  ::moab::Range moabRange = smtkToMOABRange(range);
  ::moab::Range meshes = moabRange.subset_by_type(::moab::MBENTITYSET);
  ::moab::Range cells = ::moab::subtract(moabRange, meshes);

  smtk::mesh::CellTypes ctypes;

  //compute the type of the meshes, I don't want to try and extract the range
  //of cells for all the meshes, as that could be large
  for (cit m = meshes.begin(); m != meshes.end(); ++m)
  {
    const ::moab::EntityHandle& currentHandle = *m;
    for (std::size_t i = 0; i < ctypes.size(); ++i)
    {
      const CellEnum ce = static_cast<CellEnum>(i);
      const ::moab::EntityType moabEType =
        static_cast<::moab::EntityType>(smtk::mesh::moab::smtkToMOABCell(ce));

      //some of the cell types that smtk supports moab doesn't support
      //so we can't query on those.
      int num = 0;
      m_iface->get_number_entities_by_type(
        currentHandle, static_cast<::moab::EntityType>(moabEType), num, true);
      if (num > 0)
      {
        ctypes[ce] = true;
      }
    }
  }

  //compute the type of the cells if we have any
  if (!cells.empty())
  {
    for (std::size_t i = 0; i < ctypes.size(); ++i)
    {
      //now we need to convert from CellEnum to MoabType
      const CellEnum ce = static_cast<CellEnum>(i);
      const ::moab::EntityType moabEType =
        static_cast<::moab::EntityType>(smtk::mesh::moab::smtkToMOABCell(ce));

      //if num_of_type is greater than zero we have cells of that type
      if (cells.num_of_type(moabEType) > 0)
      {
        ctypes[ce] = true;
      }
    }
  }

  const bool hasM = !(meshes.empty());
  const bool hasC = ctypes.any();
  return smtk::mesh::TypeSet(ctypes, hasM, hasC);
}

bool Interface::computeShell(const smtk::mesh::HandleRange& meshes, smtk::mesh::HandleRange& shell)
  const
{
  //step 1 get all the highest dimension cells for the meshes
  ::moab::Range cells;
  int dimension = 4;
  bool hasCells = false;
  while (!hasCells && dimension >= 0)
  {
    --dimension;
    // get all non-meshset entities in meshset of a given cell type
    for (auto i = boost::icl::elements_begin(meshes); i != boost::icl::elements_end(meshes); ++i)
    {
      //get_entities_by_dimension appends to the range given
      m_iface->get_entities_by_dimension(*i, dimension, cells, true);
    }
    hasCells = !cells.empty();
  }

  if (!hasCells)
  {
    return false;
  }

  int skinDim = dimension - 1;

  //We need to first create the adjacencies from the requested dimension to the
  //dimension of the skin. The first step is create all the adjacencies from
  //the desired dimension to the skin dimension
  ::moab::Range allAdj;
  this->moabInterface()->get_adjacencies(cells, skinDim, true, allAdj);

  ::moab::Skinner skinner(this->moabInterface());
  ::moab::Range moabShell;
  ::moab::ErrorCode rval = skinner.find_skin(this->getRoot(), cells, skinDim, moabShell);

  if (rval != ::moab::MB_SUCCESS)
  {
    //if the skin extraction failed remove all cells we created
    this->moabInterface()->delete_entities(allAdj);
  }
  else
  {
    shell = moabToSMTKRange(moabShell);
    //remove any cell created by computing the adjacencies that isn't part
    //of the skin. This is done to keep the memory utilization low
    ::moab::Range unusedCells = ::moab::subtract(allAdj, moabShell);
    this->moabInterface()->delete_entities(unusedCells);
  }

  return (rval == ::moab::MB_SUCCESS);
}

bool Interface::computeAdjacenciesOfDimension(
  const smtk::mesh::HandleRange& meshes,
  int dimension,
  smtk::mesh::HandleRange& adj) const
{
  if (dimension < smtk::mesh::Dims0 || dimension >= smtk::mesh::DimensionType_MAX)
  {
    return false;
  }

  ::moab::Range cells;
  for (auto i = boost::icl::elements_begin(meshes); i != boost::icl::elements_end(meshes); ++i)
  {
    //get_entities_by_handle appends to the range given
    m_iface->get_entities_by_handle(*i, cells, true);
  }

  ::moab::Range moabAdj;
  ::moab::ErrorCode rval = this->moabInterface()->get_adjacencies(
    cells, dimension, true, moabAdj, ::moab::Interface::UNION);
  adj = moabToSMTKRange(moabAdj);

  return (rval == ::moab::MB_SUCCESS);
}

bool Interface::canonicalIndex(
  const smtk::mesh::Handle& cellId,
  smtk::mesh::Handle& parent,
  int& canonicalIndex) const
{
  // Access the cell's parent cell
  std::vector<::moab::EntityHandle> adjacencies;
  m_iface->get_adjacencies(
    &cellId, 1, m_iface->dimension_from_handle(cellId) + 1, false, adjacencies);

  // Exit early if the cell's parent was not found
  if (adjacencies.empty())
  {
    return false;
  }

  // Assign the parent handle
  parent = adjacencies[0];

  // Access the cell's "side number" (the canonical ordering information side
  // number)
  int sense, offset;
  ::moab::ErrorCode rval = m_iface->side_number(parent, cellId, canonicalIndex, sense, offset);
  return (rval == ::moab::MB_SUCCESS);
}

bool Interface::mergeCoincidentContactPoints(
  const smtk::mesh::HandleRange& meshes,
  double tolerance)
{
  if (meshes.empty())
  {
    //I can't see a reason why we should consider a merge of nothing to be a
    //failure. So we return true.
    return true;
  }

  //we want to merge the contact points for all dimensions
  //of the meshes, not just the highest dimension i expect
  smtk::mesh::moab::MergeMeshVertices meshmerger(this->moabInterface());
  ::moab::ErrorCode rval = meshmerger.merge_entities(smtkToMOABRange(meshes), tolerance);
  if (rval == ::moab::MB_SUCCESS)
  {
    m_modified = true;
    return true;
  }
  return false;
}

smtk::mesh::HandleRange Interface::neighbors(const smtk::mesh::Handle& cellId) const
{
  int dimension = m_iface->dimension_from_handle(cellId);

  // Access the cell's boundaries
  std::vector<::moab::EntityHandle> adjacencies;
  m_iface->get_adjacencies(&cellId, 1, dimension - 1, true, adjacencies);

  // Exit early if the cell's boundaries were not found
  if (adjacencies.empty())
  {
    return smtk::mesh::HandleRange();
  }

  std::vector<::moab::EntityHandle> neighbors;
  m_iface->get_adjacencies(
    adjacencies.data(),
    static_cast<int>(adjacencies.size()),
    dimension,
    true,
    neighbors,
    ::moab::Core::UNION);

  smtk::mesh::HandleRange neighborsRange;
  for (auto& neighbor : neighbors)
  {
    neighborsRange.insert(neighbor);
  }
  neighborsRange.erase(cellId);

  return neighborsRange;
}

bool Interface::setDomain(const smtk::mesh::HandleRange& meshsets, const smtk::mesh::Domain& domain)
  const
{
  if (meshsets.empty())
  {
    return true;
  }

  tag::QueryMaterialTag mtag(domain.value(), this->moabInterface());
  bool tagged = detail::setDenseTagValues(mtag, smtkToMOABRange(meshsets), this->moabInterface());
  if (tagged)
  {
    m_modified = true;
  }
  return tagged;
}

bool Interface::setDirichlet(
  const smtk::mesh::HandleRange& meshsets,
  const smtk::mesh::Dirichlet& dirichlet) const
{
  if (meshsets.empty())
  {
    return true;
  }

  tag::QueryDirichletTag dtag(dirichlet.value(), this->moabInterface());

  //get all non-meshset entities of a given dimension
  ::moab::Range cells;
  for (auto i = boost::icl::elements_begin(meshsets); i != boost::icl::elements_end(meshsets); ++i)
  {
    //get_entities_by_dimension appends to the range given
    m_iface->get_entities_by_dimension(*i, static_cast<int>(smtk::mesh::Dims0), cells, true);
  }

  bool cellsTagged = detail::setDenseTagValues(dtag, cells, this->moabInterface());
  bool meshesTagged =
    detail::setDenseTagValues(dtag, smtkToMOABRange(meshsets), this->moabInterface());

  const bool tagged = cellsTagged && meshesTagged;
  if (tagged)
  {
    m_modified = true;
  }
  return tagged;
}

bool Interface::setNeumann(
  const smtk::mesh::HandleRange& meshsets,
  const smtk::mesh::Neumann& neumann) const
{
  if (meshsets.empty())
  {
    return true;
  }

  tag::QueryNeumannTag ntag(neumann.value(), this->moabInterface());

  //step 0 tag the meshsets
  bool tagged = detail::setDenseTagValues(ntag, smtkToMOABRange(meshsets), this->moabInterface());

  //step 1 find the highest dimension cells for the meshes.
  int dimension = 4;
  bool hasCells = false;
  while (!hasCells && dimension >= 0)
  {
    ::moab::Range cells;
    --dimension;
    for (auto i = boost::icl::elements_begin(meshsets); i != boost::icl::elements_end(meshsets);
         ++i)
    {
      m_iface->get_entities_by_dimension(*i, dimension, cells, true);
    }
    hasCells = !cells.empty();
  }

  //step 2 apply the neumann property to all cells for dimension that is 1 lower
  //since that would be the boundary dimension
  if (hasCells && dimension > 0)
  {
    ::moab::Range cells;
    --dimension;
    for (auto i = boost::icl::elements_begin(meshsets); i != boost::icl::elements_end(meshsets);
         ++i)
    {
      m_iface->get_entities_by_dimension(*i, dimension, cells, true);
    }
    tagged = tagged && detail::setDenseTagValues(ntag, cells, this->moabInterface());
  }

  if (tagged)
  {
    m_modified = true;
  }
  return tagged;
}

/**\brief Set the id for a meshset to \a id.
  */
bool Interface::setId(const smtk::mesh::Handle& meshset, const smtk::common::UUID& id) const
{
  if (!id)
  {
    return false;
  }

  tag::QueryIdTag mtag(id, this->moabInterface());

  //Tag the meshsets
  bool tagged = detail::setDenseOpaqueTagValue(mtag, meshset, this->moabInterface());
  if (tagged)
  {
    m_modified = true;
  }
  return tagged;
}

/**\brief Get the id for a meshset.
  */
smtk::common::UUID Interface::getId(const smtk::mesh::Handle& meshset) const
{
  tag::QueryIdTag mtag(this->moabInterface());
  return detail::computeDenseOpaqueTagValue<smtk::common::UUID>(
    mtag, meshset, this->moabInterface());
}

/**\brief Find a mesh entity using its id.
  *
  */
bool Interface::findById(
  const smtk::mesh::Handle& root,
  const smtk::common::UUID& id,
  smtk::mesh::Handle& meshset) const
{
  if (!id)
  {
    return false;
  }

  ::moab::Range result;

  tag::QueryIdTag mtag(id, this->moabInterface());

  ::moab::ErrorCode rval;

  rval = m_iface->get_entities_by_type_and_tag(
    root, ::moab::MBENTITYSET, mtag.moabTagPtr(), mtag.moabTagValuePtr(), 1, result);

  if (rval != ::moab::MB_SUCCESS || result.size() != 1)
  {
    // The above call does not check the root if it has the tag value. Let's do
    // that before we give up.
    smtk::common::UUID rootId =
      detail::computeDenseOpaqueTagValue<smtk::common::UUID>(mtag, root, this->moabInterface());
    if (rootId != smtk::common::UUID::null())
    {
      meshset = root;
      return true;
    }

    return false;
  }
  meshset = *result.begin();
  return true;
}

/**\brief Set the model entity assigned to each meshset member to \a ent.
  */
bool Interface::setAssociation(
  const smtk::common::UUID& modelUUID,
  const smtk::mesh::HandleRange& range) const
{
  if (range.empty() || !modelUUID)
  { //if empty range or invalid uuid
    return false;
  }

  tag::QueryEntRefTag mtag(modelUUID, this->moabInterface());

  //Tag the meshsets
  bool tagged =
    detail::setDenseOpaqueTagValues(mtag, smtkToMOABRange(range), this->moabInterface());
  if (tagged)
  {
    m_modified = true;
  }
  return tagged;
}

/**\brief Find mesh entities associated with the given model entity.
  *
  */
smtk::mesh::HandleRange Interface::findAssociations(
  const smtk::mesh::Handle& root,
  const smtk::common::UUID& modelUUID) const
{
  ::moab::Range result;
  if (!modelUUID)
  {
    return smtk::mesh::HandleRange();
  }

  tag::QueryEntRefTag mtag(modelUUID, this->moabInterface());

  ::moab::ErrorCode rval;
  rval = m_iface->get_entities_by_type_and_tag(
    root, ::moab::MBENTITYSET, mtag.moabTagPtr(), mtag.moabTagValuePtr(), 1, result);
  if (rval != ::moab::MB_SUCCESS)
  {
    result.clear();
  }
  return moabToSMTKRange(result);
}

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
  bool tagged = detail::setDenseOpaqueTagValue(mtag, root, this->moabInterface());
  if (tagged)
  {
    m_modified = true;
  }
  return tagged;
}

/// brief Get the model entity assigned to the root of this interface.
//
smtk::common::UUID Interface::rootAssociation() const
{
  //first we need to verify that we have a ROOT_MODEL tag first
  std::vector<::moab::Tag> tag_handles;
  smtk::mesh::Handle root = m_iface->get_root_set();
  m_iface->tag_get_tags_on_entity(root, tag_handles);

  if (!tag_handles.empty())
  {
    tag::QueryRootModelEntTag mtag(this->moabInterface());
    return detail::computeDenseOpaqueTagValue<smtk::common::UUID>(
      mtag, root, this->moabInterface());
  }
  return smtk::common::UUID::null();
}

//create a data set named <name> with <dimension> doubles for each cell in
//<meshsets>, and populate it with <data>
bool Interface::createCellField(
  const smtk::mesh::HandleRange& meshsets,
  const std::string& name,
  std::size_t dimension,
  const smtk::mesh::FieldType& type,
  const void* data)
{
  if (meshsets.empty())
  {
    // If there are no meshsets, then we return with failure
    return false;
  }

  // We first construct the data set for the cells associated with the meshsets.
  ::moab::Range cells;
  for (auto i = boost::icl::elements_begin(meshsets); i != boost::icl::elements_end(meshsets); ++i)
  {
    //get_entities_by_handle appends to the range given
    m_iface->get_entities_by_handle(*i, cells, true);
  }

  if (cells.empty())
  {
    // If there are no cells, then there we return with failure.
    return false;
  }
  // The double tag is used to associate double-valued data with the cells
  tag::QueryFieldTag dtag(
    name.c_str(),
    static_cast<int>(dimension),
    (type == smtk::mesh::FieldType::Integer ? ::moab::MB_TYPE_INTEGER : ::moab::MB_TYPE_DOUBLE),
    this->moabInterface());
  if (dtag.state() != ::moab::MB_SUCCESS && dtag.state() != ::moab::MB_ALREADY_ALLOCATED)
  {
    return false;
  }

  ::moab::ErrorCode rval = m_iface->tag_set_data(dtag.moabTag(), cells, data);
  bool tagged = (rval == ::moab::MB_SUCCESS);

  if (tagged)
  {
    // We successfully constructed the data set associated with the cells. Now we mark the meshset
    // as having the associated dataset. For this, we use a bit tag to simply denote the dataset's
    // existence.
    ::moab::Range moabMeshsets = smtkToMOABRange(meshsets);
    tag::QueryCellFieldTag btag(name.c_str(), this->moabInterface());
    bool* boolean_tag_values = new bool[moabMeshsets.size()];
    memset(boolean_tag_values, true, moabMeshsets.size());
    rval = m_iface->tag_set_data(btag.moabTag(), moabMeshsets, boolean_tag_values);
    assert(rval == ::moab::MB_SUCCESS);

    auto tags = this->computeCellFieldTags(*moabMeshsets.begin());

    delete[] boolean_tag_values;

    m_modified = true;
  }
  return tagged;
}

//get the dimension of a dataset.
int Interface::getCellFieldDimension(const smtk::mesh::CellFieldTag& cfTag) const
{
  ::moab::Tag tag;
  std::string dTagName = cfTag.name() + std::string("_");
  ::moab::ErrorCode rval = m_iface->tag_get_handle(dTagName.c_str(), tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return 0;
  }
  int dimension = 0;
  m_iface->tag_get_length(tag, dimension);

  return dimension;
}

//get the type of a dataset.
smtk::mesh::FieldType Interface::getCellFieldType(const smtk::mesh::CellFieldTag& cfTag) const
{
  ::moab::Tag tag;
  std::string dTagName = cfTag.name() + std::string("_");
  ::moab::ErrorCode rval = m_iface->tag_get_handle(dTagName.c_str(), tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return smtk::mesh::FieldType::MaxFieldType;
  }
  ::moab::DataType type;
  m_iface->tag_get_data_type(tag, type);

  if (type == ::moab::MB_TYPE_DOUBLE)
  {
    return smtk::mesh::FieldType::Double;
  }
  else if (type == ::moab::MB_TYPE_INTEGER)
  {
    return smtk::mesh::FieldType::Integer;
  }
  else
  {
    return smtk::mesh::FieldType::MaxFieldType;
  }
}

//find all mesh sets that have this data set
smtk::mesh::HandleRange Interface::getMeshsets(
  smtk::mesh::Handle handle,
  const smtk::mesh::CellFieldTag& cfTag) const
{
  // First access the tag associated with <name>
  ::moab::Tag tag;
  std::string name = std::string("c_") + cfTag.name();
  ::moab::ErrorCode rval = m_iface->tag_get_handle(name.c_str(), tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return smtk::mesh::HandleRange();
  }

  ::moab::Range result;

  rval =
    m_iface->get_entities_by_type_and_tag(handle, ::moab::MBENTITYSET, &tag, nullptr, 1, result);
  if (rval != ::moab::MB_SUCCESS)
  {
    result.clear();
  }
  return moabToSMTKRange(result);
}

bool Interface::hasCellField(
  const smtk::mesh::HandleRange& meshsets,
  const smtk::mesh::CellFieldTag& cfTag) const
{
  if (meshsets.empty())
  {
    // If there are no meshsets, then we return with failure
    return false;
  }

  ::moab::Range moabMeshsets = smtkToMOABRange(meshsets);

  ::moab::Tag moab_tag;
  std::string name = std::string("c_") + cfTag.name();
  ::moab::ErrorCode rval = m_iface->tag_get_handle(name.c_str(), moab_tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return false;
  }

  //fetch all entities with the given tag
  ::moab::Range entitiesWithTag;
  bool flagValue = true;
  void* flagPtr = &flagValue;
  m_iface->get_entities_by_type_and_tag(
    m_iface->get_root_set(), ::moab::MBENTITYSET, &moab_tag, &flagPtr, 1, entitiesWithTag);

  return ::moab::intersect(entitiesWithTag, moabMeshsets) == moabMeshsets;
}

bool Interface::getCellField(
  const smtk::mesh::HandleRange& meshsets,
  const smtk::mesh::CellFieldTag& cfTag,
  void* field) const
{
  if (meshsets.empty())
  {
    // If there are no meshsets, then we return with failure
    return false;
  }

  ::moab::Range cells;
  for (auto i = boost::icl::elements_begin(meshsets); i != boost::icl::elements_end(meshsets); ++i)
  {
    //get_entities_by_handle appends to the range given
    m_iface->get_entities_by_handle(*i, cells, true);
  }

  std::string dTagName = cfTag.name() + std::string("_");

  ::moab::Tag moab_tag;
  ::moab::ErrorCode rval = m_iface->tag_get_handle(dTagName.c_str(), moab_tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return false;
  }

  rval = m_iface->tag_get_data(moab_tag, cells, field);
  return (rval == ::moab::MB_SUCCESS);
}

bool Interface::setCellField(
  const smtk::mesh::HandleRange& meshsets,
  const smtk::mesh::CellFieldTag& cfTag,
  const void* const field)
{
  if (meshsets.empty())
  {
    // If there are no meshsets, then we return with failure
    return false;
  }

  ::moab::Range cells;
  for (auto i = boost::icl::elements_begin(meshsets); i != boost::icl::elements_end(meshsets); ++i)
  {
    //get_entities_by_handle appends to the range given
    m_iface->get_entities_by_handle(*i, cells, true);
  }

  std::string dTagName = cfTag.name() + std::string("_");

  ::moab::Tag moab_tag;
  ::moab::ErrorCode rval = m_iface->tag_get_handle(dTagName.c_str(), moab_tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return false;
  }

  rval = m_iface->tag_set_data(moab_tag, cells, field);

  m_modified = (rval == ::moab::MB_SUCCESS);
  return m_modified;
}

bool Interface::getField(
  const smtk::mesh::HandleRange& cells,
  const smtk::mesh::CellFieldTag& cfTag,
  void* field) const
{
  if (cells.empty())
  {
    // If there are no cells, then there we return with failure.
    return false;
  }

  std::string dTagName = cfTag.name() + std::string("_");

  ::moab::Tag moab_tag;
  ::moab::ErrorCode rval = m_iface->tag_get_handle(dTagName.c_str(), moab_tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return false;
  }

  rval = m_iface->tag_get_data(moab_tag, smtkToMOABRange(cells), field);
  return (rval == ::moab::MB_SUCCESS);
}

bool Interface::setField(
  const smtk::mesh::HandleRange& cells,
  const smtk::mesh::CellFieldTag& cfTag,
  const void* const field)
{
  if (cells.empty())
  {
    // If there are no cells, then there we return with failure.
    return false;
  }

  std::string dTagName = cfTag.name() + std::string("_");

  ::moab::Tag moab_tag;
  ::moab::ErrorCode rval = m_iface->tag_get_handle(dTagName.c_str(), moab_tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return false;
  }

  rval = m_iface->tag_set_data(moab_tag, smtkToMOABRange(cells), field);

  m_modified = (rval == ::moab::MB_SUCCESS);
  return m_modified;
}

std::set<smtk::mesh::CellFieldTag> Interface::computeCellFieldTags(
  const smtk::mesh::Handle& handle) const
{
  std::set<smtk::mesh::CellFieldTag> cellFieldTags;

  // first collect all tag handles
  std::vector<::moab::Tag> moab_tag_handles;
  m_iface->tag_get_tags(moab_tag_handles);

  for (auto& tag : moab_tag_handles)
  {
    // then filter them by type
    ::moab::DataType data_type;
    m_iface->tag_get_data_type(tag, data_type);
    if (data_type == ::moab::MB_TYPE_BIT)
    {
      // then check if there are tagged instances under our handle
      ::moab::Range range;
      ::moab::ErrorCode rval =
        m_iface->get_entities_by_type_and_tag(handle, ::moab::MBENTITYSET, &tag, nullptr, 1, range);
      if (rval == ::moab::MB_SUCCESS && !range.empty())
      {
        std::string name;
        m_iface->tag_get_name(tag, name);
        cellFieldTags.insert(smtk::mesh::CellFieldTag(name.substr(2)));
      }
    }
  }

  return cellFieldTags;
}

bool Interface::deleteCellField(
  const smtk::mesh::CellFieldTag& cfTag,
  const smtk::mesh::HandleRange& meshsets)
{
  if (meshsets.empty())
  {
    return true;
  }

  ::moab::Range cells;
  for (auto i = boost::icl::elements_begin(meshsets); i != boost::icl::elements_end(meshsets); ++i)
  {
    m_iface->get_entities_by_handle(*i, cells, true);
  }

  if (cells.empty())
  {
    // If there are no cells, then there we return with failure.
    return true;
  }

  // Access the tag associated with the cellsets
  std::string dTagName = cfTag.name() + std::string("_");
  ::moab::Tag dTag;
  ::moab::ErrorCode rval = m_iface->tag_get_handle(dTagName.c_str(), dTag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return false;
  }

  // Delete the data from the cellsets
  rval = m_iface->tag_delete_data(dTag, cells);
  if (rval != ::moab::MB_SUCCESS)
  {
    return false;
  }

  // Access the tag associated with the meshsets
  ::moab::Tag tag;
  std::string name = std::string("c_") + cfTag.name();
  rval = m_iface->tag_get_handle(name.c_str(), tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return false;
  }

  // Delete the data flag from the meshsets
  rval = m_iface->tag_delete_data(tag, smtkToMOABRange(meshsets));
  return rval == ::moab::MB_SUCCESS;
}

//create a data set named <name> with <dimension> doubles for each point in
//<meshsets>, and populate it with <data>
bool Interface::createPointField(
  const smtk::mesh::HandleRange& meshsets,
  const std::string& name,
  std::size_t dimension,
  const smtk::mesh::FieldType& type,
  const void* data)
{
  if (meshsets.empty())
  {
    // If there are no meshsets, then we return with failure
    return false;
  }

  // We first construct the data set for the points associated with the meshsets.
  ::moab::Range cells;
  for (auto i = boost::icl::elements_begin(meshsets); i != boost::icl::elements_end(meshsets); ++i)
  {
    //get_entities_by_handle appends to the range given
    m_iface->get_entities_by_handle(*i, cells, true);
  }

  ::moab::Range points;
  m_iface->get_connectivity(cells, points, false);

  if (points.empty())
  {
    // If there are no points, then there we return with failure.
    return false;
  }
  // The double tag is used to associate double-valued data with the points
  tag::QueryFieldTag dtag(
    name.c_str(),
    static_cast<int>(dimension),
    (type == smtk::mesh::FieldType::Integer ? ::moab::MB_TYPE_INTEGER : ::moab::MB_TYPE_DOUBLE),
    this->moabInterface());
  if (dtag.state() != ::moab::MB_SUCCESS && dtag.state() != ::moab::MB_ALREADY_ALLOCATED)
  {
    return false;
  }

  ::moab::ErrorCode rval = m_iface->tag_set_data(dtag.moabTag(), points, data);
  bool tagged = (rval == ::moab::MB_SUCCESS);

  if (tagged)
  {
    // We successfully constructed the data set associated with the points. Now we mark the meshset
    // as having the associated dataset. For this, we use a bit tag to simply denote the dataset's
    // existence.
    tag::QueryPointFieldTag btag(name.c_str(), this->moabInterface());
    ::moab::Range moabMeshsets = smtkToMOABRange(meshsets);
    bool* boolean_tag_values = new bool[moabMeshsets.size()];
    memset(boolean_tag_values, true, moabMeshsets.size());
    rval = m_iface->tag_set_data(btag.moabTag(), moabMeshsets, boolean_tag_values);
    assert(rval == ::moab::MB_SUCCESS);

    auto tags = this->computePointFieldTags(*moabMeshsets.begin());

    delete[] boolean_tag_values;

    m_modified = true;
  }
  return tagged;
}

//get the dimension of a dataset.
int Interface::getPointFieldDimension(const smtk::mesh::PointFieldTag& pfTag) const
{
  ::moab::Tag tag;
  std::string dTagName = pfTag.name() + std::string("_");
  ::moab::ErrorCode rval = m_iface->tag_get_handle(dTagName.c_str(), tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return 0;
  }
  int dimension = 0;
  m_iface->tag_get_length(tag, dimension);

  return dimension;
}

//get the type of a dataset.
smtk::mesh::FieldType Interface::getPointFieldType(const smtk::mesh::PointFieldTag& pfTag) const
{
  ::moab::Tag tag;
  std::string dTagName = pfTag.name() + std::string("_");
  ::moab::ErrorCode rval = m_iface->tag_get_handle(dTagName.c_str(), tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return smtk::mesh::FieldType::MaxFieldType;
  }
  ::moab::DataType type;
  m_iface->tag_get_data_type(tag, type);

  if (type == ::moab::MB_TYPE_DOUBLE)
  {
    return smtk::mesh::FieldType::Double;
  }
  else if (type == ::moab::MB_TYPE_INTEGER)
  {
    return smtk::mesh::FieldType::Integer;
  }
  else
  {
    return smtk::mesh::FieldType::MaxFieldType;
  }
}

//find all mesh sets that have this data set
smtk::mesh::HandleRange Interface::getMeshsets(
  smtk::mesh::Handle handle,
  const smtk::mesh::PointFieldTag& pfTag) const
{
  // First access the tag associated with <name>
  ::moab::Tag tag;
  std::string name = std::string("p_") + pfTag.name();
  ::moab::ErrorCode rval = m_iface->tag_get_handle(name.c_str(), tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return smtk::mesh::HandleRange();
  }

  ::moab::Range result;

  rval =
    m_iface->get_entities_by_type_and_tag(handle, ::moab::MBENTITYSET, &tag, nullptr, 1, result);
  if (rval != ::moab::MB_SUCCESS)
  {
    result.clear();
  }
  return moabToSMTKRange(result);
}

bool Interface::hasPointField(
  const smtk::mesh::HandleRange& meshsets,
  const smtk::mesh::PointFieldTag& pfTag) const
{
  if (meshsets.empty())
  {
    // If there are no meshsets, then we return with failure
    return false;
  }

  ::moab::Tag moab_tag;
  std::string name = std::string("p_") + pfTag.name();
  ::moab::ErrorCode rval = m_iface->tag_get_handle(name.c_str(), moab_tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return false;
  }

  //fetch all entities with the given tag
  ::moab::Range entitiesWithTag;
  bool flagValue = true;
  void* flagPtr = &flagValue;
  m_iface->get_entities_by_type_and_tag(
    m_iface->get_root_set(), ::moab::MBENTITYSET, &moab_tag, &flagPtr, 1, entitiesWithTag);

  ::moab::Range moabMeshsets = smtkToMOABRange(meshsets);
  return ::moab::intersect(entitiesWithTag, moabMeshsets) == moabMeshsets;
}

bool Interface::getPointField(
  const smtk::mesh::HandleRange& meshsets,
  const smtk::mesh::PointFieldTag& pfTag,
  void* field) const
{
  if (meshsets.empty())
  {
    // If there are no meshsets, then we return with failure
    return false;
  }

  ::moab::Range cells;
  for (auto i = boost::icl::elements_begin(meshsets); i != boost::icl::elements_end(meshsets); ++i)
  {
    //get_entities_by_handle appends to the range given
    m_iface->get_entities_by_handle(*i, cells, true);
  }

  ::moab::Range points;
  m_iface->get_connectivity(cells, points, false);

  std::string dTagName = pfTag.name() + std::string("_");

  ::moab::Tag moab_tag;
  ::moab::ErrorCode rval = m_iface->tag_get_handle(dTagName.c_str(), moab_tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return false;
  }

  rval = m_iface->tag_get_data(moab_tag, points, field);
  return (rval == ::moab::MB_SUCCESS);
}

bool Interface::setPointField(
  const smtk::mesh::HandleRange& meshsets,
  const smtk::mesh::PointFieldTag& pfTag,
  const void* const field)
{
  if (meshsets.empty())
  {
    // If there are no meshsets, then we return with failure
    return false;
  }

  ::moab::Range cells;
  for (auto i = boost::icl::elements_begin(meshsets); i != boost::icl::elements_end(meshsets); ++i)
  {
    //get_entities_by_handle appends to the range given
    m_iface->get_entities_by_handle(*i, cells, true);
  }

  ::moab::Range points;
  m_iface->get_connectivity(cells, points, false);

  std::string dTagName = pfTag.name() + std::string("_");

  ::moab::Tag moab_tag;
  ::moab::ErrorCode rval = m_iface->tag_get_handle(dTagName.c_str(), moab_tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return false;
  }

  rval = m_iface->tag_set_data(moab_tag, points, field);

  m_modified = (rval == ::moab::MB_SUCCESS);
  return m_modified;
}

bool Interface::getField(
  const smtk::mesh::HandleRange& points,
  const smtk::mesh::PointFieldTag& pfTag,
  void* field) const
{
  if (points.empty())
  {
    // If there are no points, then there we return with failure.
    return false;
  }

  std::string dTagName = pfTag.name() + std::string("_");

  ::moab::Tag moab_tag;
  ::moab::ErrorCode rval = m_iface->tag_get_handle(dTagName.c_str(), moab_tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return false;
  }

  rval = m_iface->tag_get_data(moab_tag, smtkToMOABRange(points), field);
  return (rval == ::moab::MB_SUCCESS);
}

bool Interface::setField(
  const smtk::mesh::HandleRange& points,
  const smtk::mesh::PointFieldTag& pfTag,
  const void* const field)
{
  if (points.empty())
  {
    // If there are no points, then there we return with failure.
    return false;
  }

  std::string dTagName = pfTag.name() + std::string("_");

  ::moab::Tag moab_tag;
  ::moab::ErrorCode rval = m_iface->tag_get_handle(dTagName.c_str(), moab_tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return false;
  }

  rval = m_iface->tag_set_data(moab_tag, smtkToMOABRange(points), field);

  m_modified = (rval == ::moab::MB_SUCCESS);
  return m_modified;
}

std::set<smtk::mesh::PointFieldTag> Interface::computePointFieldTags(
  const smtk::mesh::Handle& handle) const
{
  std::set<smtk::mesh::PointFieldTag> pointFieldTags;

  // first collect all tag handles
  std::vector<::moab::Tag> moab_tag_handles;
  m_iface->tag_get_tags(moab_tag_handles);

  for (auto& tag : moab_tag_handles)
  {
    // then filter them by type
    ::moab::DataType data_type;
    m_iface->tag_get_data_type(tag, data_type);
    if (data_type == ::moab::MB_TYPE_BIT)
    {
      // then check if there are tagged instances under our handle
      ::moab::Range range;
      ::moab::ErrorCode rval =
        m_iface->get_entities_by_type_and_tag(handle, ::moab::MBENTITYSET, &tag, nullptr, 1, range);
      if (rval == ::moab::MB_SUCCESS && !range.empty())
      {
        std::string name;
        m_iface->tag_get_name(tag, name);
        pointFieldTags.insert(smtk::mesh::PointFieldTag(name.substr(2)));
      }
    }
  }

  return pointFieldTags;
}

bool Interface::deletePointField(
  const smtk::mesh::PointFieldTag& pfTag,
  const smtk::mesh::HandleRange& meshsets)
{
  if (meshsets.empty())
  {
    return true;
  }

  ::moab::Range cells;
  for (auto i = boost::icl::elements_begin(meshsets); i != boost::icl::elements_end(meshsets); ++i)
  {
    //get_entities_by_handle appends to the range given
    m_iface->get_entities_by_handle(*i, cells, true);
  }

  ::moab::Range points;
  m_iface->get_connectivity(cells, points, false);

  if (points.empty())
  {
    // If there are no points, then there we return with failure.
    return true;
  }

  // Access the tag associated with the pointsets
  std::string dTagName = pfTag.name() + std::string("_");
  ::moab::Tag dTag;
  ::moab::ErrorCode rval = m_iface->tag_get_handle(dTagName.c_str(), dTag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return false;
  }

  // Delete the data from the pointsets
  rval = m_iface->tag_delete_data(dTag, points);
  if (rval != ::moab::MB_SUCCESS)
  {
    return false;
  }

  // Access the tag associated with the meshsets
  ::moab::Tag tag;
  std::string name = std::string("p_") + pfTag.name();
  rval = m_iface->tag_get_handle(name.c_str(), tag);
  if (rval != ::moab::MB_SUCCESS)
  {
    return false;
  }

  // Delete the data flag from the meshsets
  rval = m_iface->tag_delete_data(tag, smtkToMOABRange(meshsets));
  return rval == ::moab::MB_SUCCESS;
}

smtk::mesh::HandleRange Interface::pointIntersect(
  const smtk::mesh::HandleRange& a_,
  const smtk::mesh::HandleRange& b,
  smtk::mesh::PointConnectivity& bpc,
  smtk::mesh::ContainmentType containmentType) const
{
  if (a_.empty() || b.empty())
  { //the intersection with nothing is nothing
    return smtk::mesh::HandleRange();
  }

  ::moab::Range a = smtkToMOABRange(a_);

  //first get all the points of a
  ::moab::Range a_points = a.subset_by_type(::moab::MBVERTEX);
  m_iface->get_connectivity(a, a_points);

  if (a_points.empty())
  {
    return smtk::mesh::HandleRange();
  }

  std::vector<::moab::EntityHandle> vresult;
  if (!bpc.is_empty())
  {
    int size = 0;
    const smtk::mesh::Handle* connectivity;
    bpc.initCellTraversal();
    for (auto i = boost::icl::elements_begin(b); i != boost::icl::elements_end(b); ++i)
    {
      const bool validCell = bpc.fetchNextCell(size, connectivity);
      if (validCell)
      {
        bool exitCondition = (containmentType == smtk::mesh::PartiallyContained);
        bool contains = !exitCondition;
        for (int j = 0; j < size && contains != exitCondition; ++j)
        {
          contains = (a_points.find(connectivity[j]) != a_points.end());
        }

        if (contains)
        {
          vresult.push_back(*i);
        }
      }
    }
  }
  return detail::vectorToHandleRange(vresult);
}

smtk::mesh::HandleRange Interface::pointDifference(
  const smtk::mesh::HandleRange& a_,
  const smtk::mesh::HandleRange& b,
  smtk::mesh::PointConnectivity& bpc,
  smtk::mesh::ContainmentType containmentType) const
{
  if (a_.empty() || b.empty())
  { //the intersection with nothing is nothing
    return smtk::mesh::HandleRange();
  }

  ::moab::Range a = smtkToMOABRange(a_);

  //first get all the points of a
  ::moab::Range a_points = a.subset_by_type(::moab::MBVERTEX);
  m_iface->get_connectivity(a, a_points);

  if (a_points.empty())
  {
    return smtk::mesh::HandleRange();
  }

  std::vector<::moab::EntityHandle> vresult;
  if (!bpc.is_empty())
  {
    int size = 0;
    const smtk::mesh::Handle* connectivity;
    bpc.initCellTraversal();
    for (auto i = boost::icl::elements_begin(b); i != boost::icl::elements_end(b); ++i)
    {
      const bool validCell = bpc.fetchNextCell(size, connectivity);
      if (validCell)
      {
        bool exitCondition = (containmentType == smtk::mesh::PartiallyContained);
        bool contains = !exitCondition;
        for (int j = 0; j < size && contains != exitCondition; ++j)
        {
          contains = (a_points.find(connectivity[j]) != a_points.end());
        }

        if (!contains)
        {
          vresult.push_back(*i);
        }
      }
    }
  }
  return detail::vectorToHandleRange(vresult);
}

void Interface::callPointForEach(
  const HandleRange& points,
  std::vector<double>& coords,
  smtk::mesh::PointForEach& filter) const
{
  ::moab::Range moabPoints = smtkToMOABRange(points);

  //fetch all the coordinates
  m_iface->get_coords(moabPoints, coords.data());

  //call the filter for the rest of the points
  bool shouldBeSaved = false;
  filter.forPoints(points, coords, shouldBeSaved);
  if (shouldBeSaved)
  {
    m_iface->set_coords(moabPoints, coords.data());
  }
  return;
}

namespace
{
const std::size_t numPointsPerCall = 65536; //selected so that buffer is ~1MB
}

void Interface::pointForEach(const HandleRange& points, smtk::mesh::PointForEach& filter) const
{
  HandleRange pts = points;
  std::vector<double> coords;
  pointForEachRecursive(pts, coords, filter);
}

void Interface::pointForEachRecursive(
  HandleRange& points,
  std::vector<double>& coords,
  smtk::mesh::PointForEach& filter) const
{
  // If there are no points to call, return early
  if (points.empty())
  {
    return;
  }

  HandleRange::const_iterator it = points.begin();

  Handle begin = it->lower();
  std::size_t size = 0;

  Handle lastLower;
  HandleRange pts;

  // Iterate the intervals in the range to find the interval that contains the
  // <numPointsPerCall>-th point.
  do
  {
    lastLower = it->lower();
    Handle end = it->upper();
    size += (end - lastLower + 1);
    if (size < numPointsPerCall)
    {
      pts.insert(pts.end(), HandleInterval(lastLower, end));
    }
    else
    {
      break;
    }
  } while (++it != points.end());

  // If the number of points is fewer than <numPointsPerCall>, then we operate
  // on the entire points array. We do not perform this check earlier as it is
  // an O[n] operation where n = number of intervals. We absorb that cost into
  // the loop this algorithm uses to find the partitioning point.
  if (it == points.end())
  {
    if (coords.empty())
    {
      coords.resize(3 * size);
    }
    callPointForEach(points, coords, filter);
    return;
  }

  if (coords.empty())
  {
    coords.resize(3 * numPointsPerCall);
  }

  Handle partition = lastLower + numPointsPerCall - 1;

  pts.insert(pts.end(), HandleInterval(lastLower, partition));
  callPointForEach(pts, coords, filter);

  // Construct a range containing a single interval from the first Handle to
  // the partitioning Handle. The proceeding set operations are O[m log(n)]
  // where m is the number of intervals in this range, so it is more efficient
  // to be less accurate here.
  HandleRange toSubtract;
  toSubtract.insert(HandleInterval(begin, partition));

  points -= toSubtract;

  pointForEachRecursive(points, coords, filter);
}

void Interface::cellForEach(
  const HandleRange& cells,
  smtk::mesh::PointConnectivity& pc,
  smtk::mesh::CellForEach& filter) const
{
  if (!pc.is_empty())
  {
    smtk::mesh::CellType cellType;
    int size = 0;
    const smtk::mesh::Handle* points;

    auto currentCell = boost::icl::elements_begin(cells);
    if (filter.wantsCoordinates())
    {
      std::vector<double> coords;
      for (pc.initCellTraversal(); pc.fetchNextCell(cellType, size, points); ++currentCell)
      {
        coords.resize(size * 3);

        //query to grab the coordinates for these points
        m_iface->get_coords(points, size, coords.data());
        //call the custom filter
        filter.pointIds(points);
        filter.coordinates(&coords);
        filter.forCell(*currentCell, cellType, size);
      }
    }
    else
    { //don't extract the coords
      for (pc.initCellTraversal(); pc.fetchNextCell(cellType, size, points); ++currentCell)
      {
        filter.pointIds(points);
        //call the custom filter
        filter.forCell(*currentCell, cellType, size);
      }
    }
  }
  return;
}

void Interface::meshForEach(const smtk::mesh::HandleRange& meshes, smtk::mesh::MeshForEach& filter)
  const
{
  if (!meshes.empty())
  {
    for (auto i = boost::icl::elements_begin(meshes); i != boost::icl::elements_end(meshes); ++i)
    {

      smtk::mesh::HandleRange singleHandle;
      singleHandle += *i;
      smtk::mesh::MeshSet singleMesh(filter.m_resource, *i, singleHandle);

      //call the custom filter
      filter.forMesh(singleMesh);
    }
  }
  return;
}

bool Interface::deleteHandles(const smtk::mesh::HandleRange& toDel)
{
  //step 1. verify HandleRange isnt empty
  if (toDel.empty())
  {
    return true;
  }

  ::moab::Range moabToDel = smtkToMOABRange(toDel);

  //step 2. verify HandleRange doesn't contain root Handle
  if (moabToDel.front() == this->getRoot())
  {
    //Ranges are always sorted, and the root is always id 0
    return false;
  }

  //step 3. verify HandleRange is either all entity sets or cells/verts
  //this could be a performance bottleneck since we are using size
  bool isDeleted = false;
  if (moabToDel.all_of_type(::moab::MBENTITYSET))
  {
    //first remove any model entity relation-ship these meshes have
    tag::QueryEntRefTag mtag(this->moabInterface());
    m_iface->tag_delete_data(mtag.moabTag(), moabToDel);

    //we are all moab entity sets, fine to delete
    const ::moab::ErrorCode rval = m_iface->delete_entities(moabToDel);
    isDeleted = (rval == ::moab::MB_SUCCESS);
  }
  else if (moabToDel.num_of_type(::moab::MBENTITYSET) == 0)
  {
    //first remove any model entity relation-ship these cells have
    tag::QueryEntRefTag mtag(this->moabInterface());
    m_iface->tag_delete_data(mtag.moabTag(), moabToDel);

    //for now we are going to avoid deleting any vertex
    ::moab::Range vertCells = moabToDel.subset_by_dimension(0);
    ::moab::Range otherCells = ::moab::subtract(moabToDel, vertCells);

    //we have zero entity sets so we must be all cells/coords
    const ::moab::ErrorCode rval = m_iface->delete_entities(otherCells);

    //we don't delete the vertices, as those can't be explicitly deleted
    //instead they are deleted when the mesh goes away
    return (rval == ::moab::MB_SUCCESS);
  }
  if (isDeleted)
  {
    m_modified = true;
  }
  return isDeleted;
}

::moab::Interface* Interface::moabInterface() const
{
  return m_iface.get();
}
} // namespace moab
} // namespace mesh
} // namespace smtk
