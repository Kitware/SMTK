//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/CellField.h"
#include "smtk/mesh/core/PointField.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/core/Interface.h"

#include <cassert>

namespace smtk
{
namespace mesh
{

MeshSet::MeshSet() = default;

MeshSet::MeshSet(const smtk::mesh::ResourcePtr& parent, smtk::mesh::Handle handle)
{
  m_parent = parent;
  m_handle = handle;

  const smtk::mesh::InterfacePtr& iface = parent->interface();
  //range of entity sets
  m_range = iface->getMeshsets(handle);
}

MeshSet::MeshSet(const smtk::mesh::ConstResourcePtr& parent, smtk::mesh::Handle handle)
{
  m_parent = std::const_pointer_cast<smtk::mesh::Resource>(parent);
  m_handle = handle;

  const smtk::mesh::InterfacePtr& iface = parent->interface();
  //range of entity sets
  m_range = iface->getMeshsets(handle);
}

MeshSet::MeshSet(
  const smtk::mesh::ResourcePtr& parent,
  smtk::mesh::Handle handle,
  const smtk::mesh::HandleRange& range)
  : m_parent(parent)
  , m_handle(handle)
  , m_range(range) //range of entity sets
{
}

MeshSet::MeshSet(
  const smtk::mesh::ConstResourcePtr& parent,
  smtk::mesh::Handle handle,
  const smtk::mesh::HandleRange& range)
  : m_parent(std::const_pointer_cast<smtk::mesh::Resource>(parent))
  , m_handle(handle)
  , m_range(range) //range of entity sets
{
}

MeshSet::MeshSet(const smtk::mesh::MeshSet& other)
  : m_parent(other.m_parent)
  , m_handle(other.m_handle)
  , m_range(other.m_range)
{
}

MeshSet::~MeshSet() = default;

MeshSet& MeshSet::operator=(const MeshSet& other)
{
  m_parent = other.m_parent;
  m_handle = other.m_handle;
  m_range = other.m_range;
  return *this;
}

bool MeshSet::operator==(const MeshSet& other) const
{
  return m_parent == other.m_parent && m_handle == other.m_handle &&
    //empty is a fast way to check for easy mismatching ranges
    m_range.empty() == other.m_range.empty() && m_range == other.m_range;
}

bool MeshSet::operator!=(const MeshSet& other) const
{
  return !(*this == other);
}

bool MeshSet::operator<(const MeshSet& other) const
{
  const std::size_t myLen = m_range.size();
  const std::size_t otherLen = other.size();

  //only when the number of elements in the two meshsets are equal do
  //we need to do a complex less than comparison
  if (myLen == otherLen)
  {
    //next we look at psize which is the number of pairs inside the range
    const std::size_t myPLen = smtk::mesh::rangeIntervalCount(m_range);
    const std::size_t otherPLen = smtk::mesh::rangeIntervalCount(other.m_range);

    if (myPLen == otherPLen)
    {
      //we now have two handle ranges with same number of values, and
      //the same number of pairs. Now we need

      auto i = m_range.begin();
      auto j = other.m_range.begin();
      for (; i != m_range.end(); ++i, ++j)
      {
        if (i->lower() != j->lower())
        {
          return i->lower() < j->lower();
        }
        else if (i->upper() != j->upper())
        {
          return i->upper() < j->upper();
        }
      }
      //we looped over the entire set and everything was equal, so therefore
      //we must compare parents.

      if (m_parent && other.m_parent)
      { //If and only if the two parents exist can we safely compare
        //the uuids of the parents
        return m_parent->entity() < other.m_parent->entity();
      }
      else
      {
        //Return true when we have a non null parent
        return !!m_parent;
      }
    }

    //prefer less pair sets over more pair sets for less than operator
    return myPLen < otherPLen;
  }

  //prefer smaller lengths over larger for less than operator
  return myLen < otherLen;
}

bool MeshSet::append(const MeshSet& other)
{
  if (!m_parent)
  {
    m_parent = other.m_parent;
  }
  const bool can_append = m_parent == other.m_parent;
  if (can_append)
  {
    m_range += other.m_range;
  }
  return can_append;
}

bool MeshSet::isValid() const
{
  // A valid meshset has a valid resource and is a member of the resource.
  smtk::mesh::ResourcePtr resource = this->resource();
  if (!resource)
  {
    return false;
  }
  return !set_intersect(*this, resource->meshes()).is_empty();
}

bool MeshSet::is_empty() const
{
  return m_range.empty();
}

std::string MeshSet::name() const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  if (m_range.size() == 1)
  {
    return iface->name(smtk::mesh::rangeElement(m_range, 0));
  }
  else
  {
    return iface->name(m_handle);
  }
}

bool MeshSet::setName(const std::string& name)
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  if (m_range.size() == 1)
  {
    return iface->setName(smtk::mesh::rangeElement(m_range, 0), name);
  }
  else
  {
    return iface->setName(m_handle, name);
  }
}

std::size_t MeshSet::size() const
{
  return m_range.size();
}

std::vector<smtk::mesh::Domain> MeshSet::domains() const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->computeDomainValues(m_range);
}

std::vector<smtk::mesh::Dirichlet> MeshSet::dirichlets() const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->computeDirichletValues(m_range);
}

std::vector<smtk::mesh::Neumann> MeshSet::neumanns() const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->computeNeumannValues(m_range);
}

bool MeshSet::setDomain(const smtk::mesh::Domain& d)
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->setDomain(m_range, d);
}

bool MeshSet::setDirichlet(const smtk::mesh::Dirichlet& d)
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->setDirichlet(m_range, d);
}

bool MeshSet::setNeumann(const smtk::mesh::Neumann& n)
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->setNeumann(m_range, n);
}

/**\brief Return the meshset's UUID.
  *
  */
const smtk::common::UUID& MeshSet::id() const
{
  if (!m_id)
  {
    const smtk::mesh::InterfacePtr& iface = m_parent->interface();
    if (m_range.size() == 1)
    {
      m_id = iface->getId(smtk::mesh::rangeElement(m_range, 0));
    }
    else
    {
      m_id = iface->getId(m_handle);
    }
  }
  return m_id;
}

/**\brief Set the meshset's UUID.
  *
  */
void MeshSet::setId(const smtk::common::UUID& id)
{
  m_id = id;
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  if (m_range.size() == 1)
  {
    iface->setId(smtk::mesh::rangeElement(m_range, 0), id);
  }
  else
  {
    iface->setId(m_handle, id);
  }
}

/**\brief Return an array of model entity UUIDs associated with meshset members.
  *
  */
smtk::common::UUIDArray MeshSet::modelEntityIds() const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->computeModelEntities(m_range);
}

/**\brief Return the model entities associated with meshset members.
  *
  * warning Note that the parent resource of this meshset must have
  *         its model resource set to a valid value or the result will
  *         be an array of invalid entries.
  */
bool MeshSet::modelEntities(smtk::model::EntityRefArray& array) const
{
  smtk::model::ResourcePtr resource = m_parent->modelResource();
  smtk::common::UUIDArray uids = this->modelEntityIds();
  for (smtk::common::UUIDArray::const_iterator it = uids.begin(); it != uids.end(); ++it)
    array.push_back(smtk::model::EntityRef(resource, *it));
  return (resource != nullptr);
}

/**\brief Set the model entity for each meshset member to \a ent.
  *
  */
bool MeshSet::setModelEntity(const smtk::model::EntityRef& ent)
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->setAssociation(ent.entity(), m_range);
}

/**\brief Set the model entity for each meshset member to \a ent.
  *
  */
bool MeshSet::setModelEntityId(const smtk::common::UUID& id)
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->setAssociation(id, m_range);
}

/**\brief Get the parent resource that this meshset belongs to.
  *
  */
const smtk::mesh::ResourcePtr& MeshSet::resource() const
{
  return m_parent;
}

std::vector<std::string> MeshSet::names() const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->computeNames(m_range);
}

smtk::mesh::TypeSet MeshSet::types() const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->computeTypes(m_range);
}

smtk::mesh::CellSet MeshSet::cells() const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells(m_range);
  return smtk::mesh::CellSet(m_parent, range);
}

smtk::mesh::PointSet MeshSet::points(bool boundary_only) const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  smtk::mesh::HandleRange cells = iface->getCells(m_range);
  smtk::mesh::HandleRange range = iface->getPoints(cells, boundary_only);
  return smtk::mesh::PointSet(m_parent, range);
}

smtk::mesh::PointConnectivity MeshSet::pointConnectivity() const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells(m_range);
  return smtk::mesh::PointConnectivity(m_parent, range);
}

smtk::mesh::CellSet MeshSet::cells(smtk::mesh::CellType cellType) const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells(m_range, cellType);
  return smtk::mesh::CellSet(m_parent, range);
}

smtk::mesh::CellSet MeshSet::cells(smtk::mesh::CellTypes cellTypes) const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells(m_range, cellTypes);
  return smtk::mesh::CellSet(m_parent, range);
}

smtk::mesh::CellSet MeshSet::cells(smtk::mesh::DimensionType dim) const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells(m_range, dim);
  return smtk::mesh::CellSet(m_parent, range);
}

smtk::mesh::MeshSet MeshSet::subset(smtk::mesh::DimensionType dim) const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  smtk::mesh::HandleRange dimMeshes = iface->getMeshsets(m_handle, dim);
  //intersect our mesh id with those of a given dimension to find the subset
  return smtk::mesh::MeshSet(m_parent, m_handle, (dimMeshes & m_range));
}

smtk::mesh::MeshSet MeshSet::subset(const smtk::mesh::Domain& d) const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  smtk::mesh::HandleRange dMeshes = iface->getMeshsets(m_handle, d);
  //intersect our mesh id with those of a given dimension to find the subset
  return smtk::mesh::MeshSet(m_parent, m_handle, (dMeshes & m_range));
}

smtk::mesh::MeshSet MeshSet::subset(const smtk::mesh::Dirichlet& d) const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  smtk::mesh::HandleRange dMeshes = iface->getMeshsets(m_handle, d);
  //intersect our mesh id with those of a given dimension to find the subset
  return smtk::mesh::MeshSet(m_parent, m_handle, (dMeshes & m_range));
}

smtk::mesh::MeshSet MeshSet::subset(const smtk::mesh::Neumann& n) const
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  smtk::mesh::HandleRange nMeshes = iface->getMeshsets(m_handle, n);
  //intersect our mesh id with those of a given dimension to find the subset
  return smtk::mesh::MeshSet(m_parent, m_handle, (nMeshes & m_range));
}

smtk::mesh::MeshSet MeshSet::subset(std::size_t ith) const
{
  smtk::mesh::HandleRange singleHandleRange;
  if (!m_range.empty() && ith < m_range.size())
  {
    singleHandleRange.insert(smtk::mesh::rangeElement(m_range, ith));
  }
  smtk::mesh::MeshSet singleMesh(m_parent, m_handle, singleHandleRange);
  return singleMesh;
}

smtk::mesh::MeshSet MeshSet::extractShell() const
{
  bool created;
  return this->extractShell(created);
}

smtk::mesh::MeshSet MeshSet::extractShell(bool& created) const
{
  created = false;
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();

  smtk::mesh::HandleRange entities;
  smtk::mesh::HandleRange cells;
  const bool shellExtracted = iface->computeShell(m_range, cells);
  if (shellExtracted)
  {
    smtk::mesh::Handle meshSetHandle;
    //create a mesh for these cells since they don't have a meshset currently
    created = iface->createMesh(cells, meshSetHandle);
    if (created)
    {
      entities.insert(meshSetHandle);
    }
  }
  return smtk::mesh::MeshSet(m_parent, m_handle, entities);
}

smtk::mesh::MeshSet MeshSet::extractAdjacenciesOfDimension(int dimension) const
{
  bool created;
  return this->extractAdjacenciesOfDimension(dimension, created);
}

smtk::mesh::MeshSet MeshSet::extractAdjacenciesOfDimension(int dimension, bool& created) const
{
  created = false;
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();

  smtk::mesh::HandleRange entities;
  smtk::mesh::HandleRange cells;
  const bool adjacenciesExtracted = iface->computeAdjacenciesOfDimension(m_range, dimension, cells);
  if (adjacenciesExtracted)
  {
    smtk::mesh::Handle meshSetHandle;
    //create a mesh for these cells since they don't have a meshset currently
    created = iface->createMesh(cells, meshSetHandle);
    if (created)
    {
      entities.insert(meshSetHandle);
    }
  }
  return smtk::mesh::MeshSet(m_parent, m_handle, entities);
}

bool MeshSet::mergeCoincidentContactPoints(double tolerance)
{
  const smtk::mesh::InterfacePtr& iface = m_parent->interface();
  return iface->mergeCoincidentContactPoints(m_range, tolerance);
}

smtk::mesh::CellField MeshSet::createCellField(
  const std::string& name,
  int dimension,
  const smtk::mesh::FieldType& type,
  const void* const data)
{
  if (name.empty() || dimension <= 0)
  {
    // CellFields must be named and have dimension higher than zero.
    return CellField();
  }

  const smtk::mesh::InterfacePtr& iface = this->resource()->interface();
  if (!iface)
  {
    return CellField();
  }

  bool success;
  if (data != nullptr)
  {
    success = iface->createCellField(m_range, name, dimension, type, data);
  }
  else
  {
    std::vector<double> tmp(this->cells().size() * dimension, 0.);
    success = iface->createCellField(m_range, name, dimension, type, tmp.data());
  }
  return success ? CellField(*this, name) : CellField();
}

smtk::mesh::CellField MeshSet::cellField(const std::string& name) const
{
  return CellField(*this, name);
}

std::set<smtk::mesh::CellField> MeshSet::cellFields() const
{
  std::set<smtk::mesh::CellField> cellfields;

  const smtk::mesh::InterfacePtr& iface = this->resource()->interface();
  if (!iface)
  {
    return cellfields;
  }

  std::set<smtk::mesh::CellFieldTag> tags = iface->computeCellFieldTags(m_handle);
  for (const auto& tag : tags)
  {
    if (iface->hasCellField(this->range(), tag))
    {
      cellfields.insert(CellField(*this, tag.name()));
    }
  }

  return cellfields;
}

bool MeshSet::removeCellField(smtk::mesh::CellField cellfield)
{
  const smtk::mesh::InterfacePtr& iface = this->resource()->interface();

  return iface->deleteCellField(CellFieldTag(cellfield.name()), m_range);
}

smtk::mesh::PointField MeshSet::createPointField(
  const std::string& name,
  int dimension,
  const smtk::mesh::FieldType& type,
  const void* const data)
{
  if (name.empty() || dimension <= 0)
  {
    // PointFields must be named and have dimension higher than zero.
    return PointField();
  }

  const smtk::mesh::InterfacePtr& iface = this->resource()->interface();
  if (!iface)
  {
    return PointField();
  }

  bool success;
  if (data != nullptr)
  {
    success = iface->createPointField(m_range, name, dimension, type, data);
  }
  else
  {
    std::vector<double> tmp(this->points().size() * dimension, 0.);
    success = iface->createPointField(m_range, name, dimension, type, tmp.data());
  }
  return success ? PointField(*this, name) : PointField();
}

smtk::mesh::PointField MeshSet::pointField(const std::string& name) const
{
  return PointField(*this, name);
}

std::set<smtk::mesh::PointField> MeshSet::pointFields() const
{
  std::set<smtk::mesh::PointField> pointfields;

  const smtk::mesh::InterfacePtr& iface = this->resource()->interface();
  if (!iface)
  {
    return pointfields;
  }

  std::set<smtk::mesh::PointFieldTag> tags = iface->computePointFieldTags(m_handle);
  for (const auto& tag : tags)
  {
    if (iface->hasPointField(this->range(), tag))
    {
      pointfields.insert(PointField(*this, tag.name()));
    }
  }

  return pointfields;
}

bool MeshSet::removePointField(smtk::mesh::PointField pointfield)
{
  const smtk::mesh::InterfacePtr& iface = this->resource()->interface();

  return iface->deletePointField(PointFieldTag(pointfield.name()), m_range);
}

//intersect two mesh sets, placing the results in the return mesh set
MeshSet set_intersect(const MeshSet& a, const MeshSet& b)
{
  if (a.m_parent != b.m_parent)
  { //return an empty MeshSet if the resources don't match
    return smtk::mesh::MeshSet(a.m_parent, a.m_handle, smtk::mesh::HandleRange());
  }

  smtk::mesh::HandleRange result = a.m_range & b.m_range;
  return smtk::mesh::MeshSet(a.m_parent, a.m_handle, result);
}

//subtract mesh b from a, placing the results in the return mesh set
MeshSet set_difference(const MeshSet& a, const MeshSet& b)
{
  if (a.m_parent != b.m_parent)
  { //return an empty MeshSet if the resources don't match
    return smtk::mesh::MeshSet(a.m_parent, a.m_handle, smtk::mesh::HandleRange());
  }

  smtk::mesh::HandleRange result = a.m_range - b.m_range;
  return smtk::mesh::MeshSet(a.m_parent, a.m_handle, result);
}

//union two mesh sets, placing the results in the return mesh set
MeshSet set_union(const MeshSet& a, const MeshSet& b)
{
  if (a.m_parent != b.m_parent)
  { //return an empty MeshSet if the resources don't match
    return smtk::mesh::MeshSet(a.m_parent, a.m_handle, smtk::mesh::HandleRange());
  }

  smtk::mesh::HandleRange result = a.m_range | b.m_range;
  return smtk::mesh::MeshSet(a.m_parent, a.m_handle, result);
}

SMTKCORE_EXPORT void for_each(const MeshSet& a, MeshForEach& filter)
{
  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();

  filter.m_resource = a.m_parent;
  iface->meshForEach(a.m_range, filter);
}
} // namespace mesh
} // namespace smtk
