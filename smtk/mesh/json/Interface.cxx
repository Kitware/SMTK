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

#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/QueryTypes.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/moab/CellTypeToType.h"
#include "smtk/mesh/moab/HandleRangeToRange.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <set>

namespace smtk
{
namespace mesh
{
namespace json
{

//construct an empty interface instance
smtk::mesh::json::InterfacePtr make_interface()
{
  return std::make_shared<smtk::mesh::json::Interface>();
}

Interface::Interface()
  : m_associated_model(smtk::common::UUID::null())
{
}

Interface::Interface(const std::vector<smtk::mesh::json::MeshInfo>& info)
  : m_meshInfo(info)
  , m_associated_model(smtk::common::UUID::null())
{
}

Interface::~Interface() = default;

bool Interface::isModified() const
{
  return m_modified;
}

void Interface::addMeshes(const std::vector<smtk::mesh::json::MeshInfo>& info)
{
  m_meshInfo.insert(m_meshInfo.end(), info.begin(), info.end());
}

smtk::mesh::AllocatorPtr Interface::allocator()
{
  return smtk::mesh::AllocatorPtr();
}

smtk::mesh::BufferedCellAllocatorPtr Interface::bufferedCellAllocator()
{
  return smtk::mesh::BufferedCellAllocatorPtr();
}

smtk::mesh::IncrementalAllocatorPtr Interface::incrementalAllocator()
{
  return smtk::mesh::IncrementalAllocatorPtr();
}

smtk::mesh::ConnectivityStoragePtr Interface::connectivityStorage(
  const smtk::mesh::HandleRange& /*cells*/)
{
  return smtk::mesh::ConnectivityStoragePtr();
}

smtk::mesh::PointLocatorImplPtr Interface::pointLocator(const smtk::mesh::HandleRange& /*points*/)
{
  return smtk::mesh::PointLocatorImplPtr();
}

smtk::mesh::PointLocatorImplPtr Interface::pointLocator(
  std::size_t /*numPoints*/,
  const std::function<std::array<double, 3>(std::size_t)>& /*coordinates*/)
{
  return smtk::mesh::PointLocatorImplPtr();
}

void Interface::registerQueries(smtk::mesh::Resource&) const {}

smtk::mesh::Handle Interface::getRoot() const
{
  return smtk::mesh::Handle(0);
}

bool Interface::createMesh(
  const smtk::mesh::HandleRange& /*cells*/,
  smtk::mesh::Handle& /*meshHandle*/)
{
  //this interface can't create new meshes
  return false;
}

std::size_t Interface::numMeshes(smtk::mesh::Handle handle) const
{
  if (handle != this->getRoot())
  {
    return 0;
  }
  return m_meshInfo.size();
}

smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle) const
{
  if (handle != this->getRoot())
  {
    return smtk::mesh::HandleRange();
  }

  smtk::mesh::HandleRange meshes;
  MeshInfoVecType::const_iterator i;
  for (i = m_meshInfo.begin(); i != m_meshInfo.end(); ++i)
  {
    meshes.insert(i->mesh());
  }
  return meshes;
}

smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle, int dimension) const
{
  if (handle != this->getRoot())
  {
    return smtk::mesh::HandleRange();
  }

  const smtk::mesh::DimensionType dim = static_cast<smtk::mesh::DimensionType>(dimension);

  smtk::mesh::HandleRange meshes;
  MeshInfoVecType::const_iterator i;
  for (i = m_meshInfo.begin(); i != m_meshInfo.end(); ++i)
  {
    if (i->types().hasDimension(dim))
    {
      meshes.insert(i->mesh());
    }
  }
  return meshes;
}

//find all entity sets that have this exact name tag
smtk::mesh::HandleRange Interface::getMeshsets(
  smtk::mesh::Handle /*handle*/,
  const std::string& /*name*/) const
{
  return smtk::mesh::HandleRange();
}

//find all entity sets that have this exact name tag
smtk::mesh::HandleRange Interface::getMeshsets(
  smtk::mesh::Handle handle,
  const smtk::mesh::Domain& domain) const
{
  if (handle != this->getRoot())
  {
    return smtk::mesh::HandleRange();
  }

  smtk::mesh::HandleRange meshes;
  MeshInfoVecType::const_iterator i;
  for (i = m_meshInfo.begin(); i != m_meshInfo.end(); ++i)
  {
    if (i->has(domain))
    {
      meshes.insert(i->mesh());
    }
  }
  return meshes;
}

//find all entity sets that have this exact name tag
smtk::mesh::HandleRange Interface::getMeshsets(
  smtk::mesh::Handle handle,
  const smtk::mesh::Dirichlet& dirichlet) const
{
  if (handle != this->getRoot())
  {
    return smtk::mesh::HandleRange();
  }

  smtk::mesh::HandleRange meshes;
  MeshInfoVecType::const_iterator i;
  for (i = m_meshInfo.begin(); i != m_meshInfo.end(); ++i)
  {
    if (i->has(dirichlet))
    {
      meshes.insert(i->mesh());
    }
  }
  return meshes;
}

//find all entity sets that have this exact name tag
smtk::mesh::HandleRange Interface::getMeshsets(
  smtk::mesh::Handle handle,
  const smtk::mesh::Neumann& neumann) const
{
  if (handle != this->getRoot())
  {
    return smtk::mesh::HandleRange();
  }

  smtk::mesh::HandleRange meshes;
  MeshInfoVecType::const_iterator i;
  for (i = m_meshInfo.begin(); i != m_meshInfo.end(); ++i)
  {
    if (i->has(neumann))
    {
      meshes.insert(i->mesh());
    }
  }
  return meshes;
}

//get all cells held by this range
smtk::mesh::HandleRange Interface::getCells(const HandleRange& meshsets) const
{
  smtk::mesh::HandleRange cells;
  for (auto i = smtk::mesh::rangeElementsBegin(meshsets);
       i != smtk::mesh::rangeElementsEnd(meshsets);
       ++i)
  {
    MeshInfoVecType::const_iterator m = this->find(*i);
    if (m != m_meshInfo.end())
    {
      cells += m->cells();
    }
  }
  return cells;
}

//get all cells held by this range handle of a given cell type
smtk::mesh::HandleRange Interface::getCells(
  const HandleRange& meshsets,
  smtk::mesh::CellType cellType) const
{
  smtk::mesh::HandleRange cells;
  for (auto i = smtk::mesh::rangeElementsBegin(meshsets);
       i != smtk::mesh::rangeElementsEnd(meshsets);
       ++i)
  {
    MeshInfoVecType::const_iterator m = this->find(*i);
    if (m != m_meshInfo.end())
    {
      cells += m->cells(cellType);
    }
  }
  return cells;
}

//get all cells held by this range handle of a given cell type(s)
smtk::mesh::HandleRange Interface::getCells(
  const smtk::mesh::HandleRange& meshsets,
  const smtk::mesh::CellTypes& cellTypes) const
{
  smtk::mesh::HandleRange cells;
  for (auto i = smtk::mesh::rangeElementsBegin(meshsets);
       i != smtk::mesh::rangeElementsEnd(meshsets);
       ++i)
  {
    MeshInfoVecType::const_iterator m = this->find(*i);
    if (m != m_meshInfo.end())
    {
      cells += m->cells(cellTypes);
    }
  }
  return cells;
}

//get all cells held by this range handle of a given dimension
smtk::mesh::HandleRange Interface::getCells(
  const smtk::mesh::HandleRange& meshsets,
  smtk::mesh::DimensionType dim) const
{
  smtk::mesh::HandleRange cells;
  for (auto i = smtk::mesh::rangeElementsBegin(meshsets);
       i != smtk::mesh::rangeElementsEnd(meshsets);
       ++i)
  {
    MeshInfoVecType::const_iterator m = this->find(*i);
    if (m != m_meshInfo.end())
    {
      cells += m->cells(dim);
    }
  }
  return cells;
}

//get all points held by this range handle
smtk::mesh::HandleRange Interface::getPoints(
  const smtk::mesh::HandleRange& cells,
  bool boundary_only) const
{
  // We don't support modifying operations via the JSON interface (like meshset
  // construction, point merging, etc.); in keeping with this paradigm, the only
  // point ranges that can be queried are unions of points associated with mesh
  // sets that have already been computed using an alternate backend (e.g.
  // Moab). Any queries involving cells that are not the union of cell sets
  // contained within the meshInfo vector yield a null return value. This is
  // sufficient for relaying server-side information about points to the client.

  (void)boundary_only;

  smtk::mesh::HandleRange points;
  smtk::mesh::HandleRange cellsToVisit = cells;
  for (const auto& m : m_meshInfo)
  {
    if (smtk::mesh::rangeContains(cellsToVisit, m.cells()))
    {
      points += m.points();
      cellsToVisit = cellsToVisit - m.cells();
    }
  }
  return cellsToVisit.empty() ? points : smtk::mesh::HandleRange();
}

bool Interface::getCoordinates(const smtk::mesh::HandleRange& /*points*/, double* /*xyz*/) const
{
  return false;
}

bool Interface::getCoordinates(const smtk::mesh::HandleRange& /*points*/, float* /*xyz*/) const
{
  return false;
}

bool Interface::setCoordinates(
  const smtk::mesh::HandleRange& /*points*/,
  const double* const /*xyz*/)
{
  return false;
}

bool Interface::setCoordinates(
  const smtk::mesh::HandleRange& /*points*/,
  const float* const /*xyz*/)
{
  return false;
}

std::string Interface::name(const smtk::mesh::Handle& /*meshset*/) const
{
  //TODO
  return std::string();
}

bool Interface::setName(const smtk::mesh::Handle& /*meshset*/, const std::string& /*name*/)
{
  return false;
}

std::vector<std::string> Interface::computeNames(const smtk::mesh::HandleRange& /*meshsets*/) const
{
  return std::vector<std::string>();
}

std::vector<smtk::mesh::Domain> Interface::computeDomainValues(
  const smtk::mesh::HandleRange& meshsets) const
{
  std::set<smtk::mesh::Domain> domains;
  for (auto i = smtk::mesh::rangeElementsBegin(meshsets);
       i != smtk::mesh::rangeElementsEnd(meshsets);
       ++i)
  {
    MeshInfoVecType::const_iterator m = this->find(*i);
    const std::vector<smtk::mesh::Domain>& t = m->domains();
    domains.insert(t.begin(), t.end());
  }

  //return a vector of all the unique domains we have
  return std::vector<smtk::mesh::Domain>(domains.begin(), domains.end());
}

std::vector<smtk::mesh::Dirichlet> Interface::computeDirichletValues(
  const smtk::mesh::HandleRange& meshsets) const
{
  std::set<smtk::mesh::Dirichlet> boundary;
  for (auto i = smtk::mesh::rangeElementsBegin(meshsets);
       i != smtk::mesh::rangeElementsEnd(meshsets);
       ++i)
  {
    MeshInfoVecType::const_iterator m = this->find(*i);
    const std::vector<smtk::mesh::Dirichlet>& t = m->dirichlets();
    boundary.insert(t.begin(), t.end());
  }

  //return a vector of all the unique Dirichlet we have
  return std::vector<smtk::mesh::Dirichlet>(boundary.begin(), boundary.end());
}

std::vector<smtk::mesh::Neumann> Interface::computeNeumannValues(
  const smtk::mesh::HandleRange& meshsets) const
{
  std::set<smtk::mesh::Neumann> boundary;
  for (auto i = smtk::mesh::rangeElementsBegin(meshsets);
       i != smtk::mesh::rangeElementsEnd(meshsets);
       ++i)
  {
    MeshInfoVecType::const_iterator m = this->find(*i);
    const std::vector<smtk::mesh::Neumann>& t = m->neumanns();
    boundary.insert(t.begin(), t.end());
  }

  //return a vector of all the unique Neumann we have
  return std::vector<smtk::mesh::Neumann>(boundary.begin(), boundary.end());
}

/**\brief Return the set of all UUIDs set on all entities in the meshsets.
  *
  */
smtk::common::UUIDArray Interface::computeModelEntities(
  const smtk::mesh::HandleRange& meshsets) const
{
  smtk::common::UUIDArray uuids;
  for (auto i = smtk::mesh::rangeElementsBegin(meshsets);
       i != smtk::mesh::rangeElementsEnd(meshsets);
       ++i)
  {
    MeshInfoVecType::const_iterator m = this->find(*i);
    if (m == m_meshInfo.end())
      continue;
    const smtk::common::UUIDArray& t = m->modelUUIDS();
    if (!t.empty())
    {
      uuids.insert(uuids.end(), t.begin(), t.end());
    }
  }
  return uuids;
}

smtk::mesh::TypeSet Interface::computeTypes(const smtk::mesh::HandleRange& range) const
{
  typedef ::smtk::mesh::CellType CellEnum;

  ::moab::Range moabRange = smtk::mesh::moab::smtkToMOABRange(range);

  ::moab::Range meshes = moabRange.subset_by_type(::moab::MBENTITYSET);
  ::moab::Range cells = ::moab::subtract(moabRange, meshes);

  smtk::mesh::TypeSet result;
  for (::moab::Range::const_iterator i = meshes.begin(); i != meshes.end(); ++i)
  {
    MeshInfoVecType::const_iterator m = this->find(*i);
    result += m->types();
  }

  smtk::mesh::CellTypes ctypes;

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
  smtk::mesh::TypeSet cellResult(ctypes, hasM, hasC);

  result += cellResult;
  return result;
}

bool Interface::computeShell(
  const smtk::mesh::HandleRange& /*meshes*/,
  smtk::mesh::HandleRange& /*shell*/) const
{
  return false;
}

bool Interface::computeAdjacenciesOfDimension(
  const smtk::mesh::HandleRange& /*meshes*/,
  int /*dimension*/,
  smtk::mesh::HandleRange& /*adj*/) const
{
  return false;
}

bool Interface::canonicalIndex(
  const smtk::mesh::Handle& /*cell*/,
  smtk::mesh::Handle& /*parent*/,
  int& /*index*/) const
{
  return false;
}

bool Interface::mergeCoincidentContactPoints(
  const smtk::mesh::HandleRange& /*meshes*/,
  double /*tolerance*/)
{
  return false;
}

smtk::mesh::HandleRange Interface::neighbors(const smtk::mesh::Handle& cellId) const
{
  (void)cellId;
  return smtk::mesh::HandleRange();
}

bool Interface::setDomain(
  const smtk::mesh::HandleRange& /*meshsets*/,
  const smtk::mesh::Domain& /*domain*/) const
{
  return false;
}

bool Interface::setDirichlet(
  const smtk::mesh::HandleRange& /*meshsets*/,
  const smtk::mesh::Dirichlet& /*dirichlet*/) const
{
  return false;
}

bool Interface::setNeumann(
  const smtk::mesh::HandleRange& /*meshsets*/,
  const smtk::mesh::Neumann& /*neumann*/) const
{
  return false;
}

bool Interface::setId(const smtk::mesh::Handle& /*meshset*/, const smtk::common::UUID& /*id*/) const
{
  return false;
}

smtk::common::UUID Interface::getId(const smtk::mesh::Handle& meshset) const
{
  smtk::mesh::HandleRange meshes;
  MeshInfoVecType::const_iterator i;
  for (i = m_meshInfo.begin(); i != m_meshInfo.end(); ++i)
  {
    if (i->mesh() == meshset)
    {
      return i->id();
    }
  }
  return smtk::common::UUID::null();
}

bool Interface::findById(
  const smtk::mesh::Handle& /*root*/,
  const smtk::common::UUID& id,
  smtk::mesh::Handle& meshset) const
{
  smtk::mesh::HandleRange meshes;
  MeshInfoVecType::const_iterator i;
  for (i = m_meshInfo.begin(); i != m_meshInfo.end(); ++i)
  {
    if (i->id() == id)
    {
      meshset = i->mesh();
      return true;
    }
  }
  return false;
}

/**\brief Set the model entity assigned to each meshset member to \a ent.
  */
bool Interface::setAssociation(
  const smtk::common::UUID& /*modelUUID*/,
  const smtk::mesh::HandleRange& /*meshsets*/) const
{
  return false;
}

/**\brief Find mesh entities associated with the given model entity.
  *
  */
smtk::mesh::HandleRange Interface::findAssociations(
  const smtk::mesh::Handle& /*root*/,
  const smtk::common::UUID& modelUUID) const
{
  smtk::mesh::HandleRange result;
  if (!modelUUID)
    return result;

  smtk::mesh::HandleRange meshes;
  MeshInfoVecType::const_iterator i;
  for (i = m_meshInfo.begin(); i != m_meshInfo.end(); ++i)
  {
    const smtk::common::UUIDArray& uuids = i->modelUUIDS();
    const bool contains = std::find(uuids.begin(), uuids.end(), modelUUID) != uuids.end();
    if (contains)
    {
      meshes.insert(i->mesh());
    }
  }
  return meshes;
}

/**\brief Set the model entity assigned to the root of this interface.
  */
bool Interface::setRootAssociation(const smtk::common::UUID& modelUUID) const
{
  m_associated_model = modelUUID;
  return true;
}

/**\brief Get the model entity assigned to the root of this interface.
  */
smtk::common::UUID Interface::rootAssociation() const
{
  return m_associated_model;
}

bool Interface::createCellField(
  const smtk::mesh::HandleRange& /*meshsets*/,
  const std::string& /*name*/,
  std::size_t /*dimension*/,
  const smtk::mesh::FieldType& /*type*/,
  const void* /*data*/)
{
  return false;
}

int Interface::getCellFieldDimension(const smtk::mesh::CellFieldTag& /*cfTag*/) const
{
  return 0;
}

smtk::mesh::FieldType Interface::getCellFieldType(const smtk::mesh::CellFieldTag& /*cfTag*/) const
{
  return smtk::mesh::FieldType::MaxFieldType;
}

smtk::mesh::HandleRange Interface::getMeshsets(
  smtk::mesh::Handle /*handle*/,
  const smtk::mesh::CellFieldTag& /*cfTag*/) const
{
  return smtk::mesh::HandleRange();
}

bool Interface::hasCellField(
  const smtk::mesh::HandleRange& /*meshsets*/,
  const smtk::mesh::CellFieldTag& /*cfTag*/) const
{
  return false;
}

bool Interface::getCellField(
  const smtk::mesh::HandleRange& /*meshsets*/,
  const smtk::mesh::CellFieldTag& /*cfTag*/,
  void* /*data*/) const
{
  return false;
}

bool Interface::setCellField(
  const smtk::mesh::HandleRange& /*meshsets*/,
  const smtk::mesh::CellFieldTag& /*cfTag*/,
  const void* const /*data*/)
{
  return false;
}

bool Interface::getField(
  const smtk::mesh::HandleRange& /*cells*/,
  const smtk::mesh::CellFieldTag& /*cfTag*/,
  void* /*data*/) const
{
  return false;
}

bool Interface::setField(
  const smtk::mesh::HandleRange& /*cells*/,
  const smtk::mesh::CellFieldTag& /*cfTag*/,
  const void* const /*data*/)
{
  return false;
}

std::set<smtk::mesh::CellFieldTag> Interface::computeCellFieldTags(
  const smtk::mesh::Handle& /*handle*/) const
{
  return std::set<smtk::mesh::CellFieldTag>();
}

bool Interface::deleteCellField(
  const smtk::mesh::CellFieldTag& /*dsTag*/,
  const smtk::mesh::HandleRange& /*meshsets*/)
{
  return false;
}

bool Interface::createPointField(
  const smtk::mesh::HandleRange& /*meshsets*/,
  const std::string& /*name*/,
  std::size_t /*dimension*/,
  const smtk::mesh::FieldType& /*type*/,
  const void* /*data*/)
{
  return false;
}

int Interface::getPointFieldDimension(const smtk::mesh::PointFieldTag& /*pfTag*/) const
{
  return 0;
}

smtk::mesh::FieldType Interface::getPointFieldType(const smtk::mesh::PointFieldTag& /*pfTag*/) const
{
  return smtk::mesh::FieldType::MaxFieldType;
}

smtk::mesh::HandleRange Interface::getMeshsets(
  smtk::mesh::Handle /*handle*/,
  const smtk::mesh::PointFieldTag& /*pfTag*/) const
{
  return smtk::mesh::HandleRange();
}

bool Interface::hasPointField(
  const smtk::mesh::HandleRange& /*meshsets*/,
  const smtk::mesh::PointFieldTag& /*pfTag*/) const
{
  return false;
}

bool Interface::getPointField(
  const smtk::mesh::HandleRange& /*meshsets*/,
  const smtk::mesh::PointFieldTag& /*pfTag*/,
  void* /*data*/) const
{
  return false;
}

bool Interface::setPointField(
  const smtk::mesh::HandleRange& /*meshsets*/,
  const smtk::mesh::PointFieldTag& /*pfTag*/,
  const void* /*data*/)
{
  return false;
}

bool Interface::getField(
  const smtk::mesh::HandleRange& /*points*/,
  const smtk::mesh::PointFieldTag& /*pfTag*/,
  void* /*data*/) const
{
  return false;
}

bool Interface::setField(
  const smtk::mesh::HandleRange& /*points*/,
  const smtk::mesh::PointFieldTag& /*pfTag*/,
  const void* /*data*/)
{
  return false;
}

std::set<smtk::mesh::PointFieldTag> Interface::computePointFieldTags(
  const smtk::mesh::Handle& /*handle*/) const
{
  return std::set<smtk::mesh::PointFieldTag>();
}

bool Interface::deletePointField(
  const smtk::mesh::PointFieldTag& /*dsTag*/,
  const smtk::mesh::HandleRange& /*meshsets*/)
{
  return false;
}

smtk::mesh::HandleRange Interface::pointIntersect(
  const smtk::mesh::HandleRange& /*a*/,
  const smtk::mesh::HandleRange& /*b*/,
  smtk::mesh::PointConnectivity& /*bpc*/,
  smtk::mesh::ContainmentType /*containmentType*/) const
{
  return smtk::mesh::HandleRange();
}

smtk::mesh::HandleRange Interface::pointDifference(
  const smtk::mesh::HandleRange& /*a*/,
  const smtk::mesh::HandleRange& /*b*/,
  smtk::mesh::PointConnectivity& /*bpc*/,
  smtk::mesh::ContainmentType /*containmentType*/) const
{
  return smtk::mesh::HandleRange();
}

void Interface::pointForEach(const HandleRange& /*points*/, smtk::mesh::PointForEach& /*filter*/)
  const
{
}

void Interface::cellForEach(
  const HandleRange& /*cells*/,
  smtk::mesh::PointConnectivity& /*a*/,
  smtk::mesh::CellForEach& /*filter*/) const
{
}

void Interface::meshForEach(
  const smtk::mesh::HandleRange& /*meshes*/,
  smtk::mesh::MeshForEach& /*filter*/) const
{
}

bool Interface::deleteHandles(const smtk::mesh::HandleRange& /*toDel*/)
{
  return false;
}

Interface::MeshInfoVecType::const_iterator Interface::find(smtk::mesh::Handle handle) const
{
  MeshInfoVecType::const_iterator result = std::find(m_meshInfo.begin(), m_meshInfo.end(), handle);
  return result;
}
} // namespace json
} // namespace mesh
} // namespace smtk
