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
#include "smtk/mesh/CellField.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/PointField.h"

#include "smtk/mesh/Interface.h"

#include <cassert>

namespace smtk
{
namespace mesh
{

MeshSet::MeshSet()
  : m_parent()
  , m_handle()
  , m_range()
{
  //Trying to make Shitbroken happy
}

MeshSet::MeshSet(const smtk::mesh::CollectionPtr& parent, smtk::mesh::Handle handle)
{
  this->m_parent = parent;
  this->m_handle = handle;

  const smtk::mesh::InterfacePtr& iface = parent->interface();
  //range of moab entity sets
  this->m_range = iface->getMeshsets(handle);
}

MeshSet::MeshSet(const smtk::mesh::ConstCollectionPtr& parent, smtk::mesh::Handle handle)
{
  this->m_parent = std::const_pointer_cast<smtk::mesh::Collection>(parent);
  this->m_handle = handle;

  const smtk::mesh::InterfacePtr& iface = parent->interface();
  //range of moab entity sets
  this->m_range = iface->getMeshsets(handle);
}

MeshSet::MeshSet(const smtk::mesh::CollectionPtr& parent, smtk::mesh::Handle handle,
  const smtk::mesh::HandleRange& range)
  : m_parent(parent)
  , m_handle(handle)
  , m_range(range) //range of moab entity sets
{
}

MeshSet::MeshSet(const smtk::mesh::ConstCollectionPtr& parent, smtk::mesh::Handle handle,
  const smtk::mesh::HandleRange& range)
  : m_parent(std::const_pointer_cast<smtk::mesh::Collection>(parent))
  , m_handle(handle)
  , m_range(range) //range of moab entity sets
{
}

MeshSet::MeshSet(const smtk::mesh::MeshSet& other)
  : m_parent(other.m_parent)
  , m_handle(other.m_handle)
  , m_range(other.m_range)
{
}

MeshSet::~MeshSet()
{
}

MeshSet& MeshSet::operator=(const MeshSet& other)
{
  this->m_parent = other.m_parent;
  this->m_handle = other.m_handle;
  this->m_range = other.m_range;
  return *this;
}

bool MeshSet::operator==(const MeshSet& other) const
{
  return this->m_parent == other.m_parent && this->m_handle == other.m_handle &&
    //empty is a fast way to check for easy mismatching ranges
    this->m_range.empty() == other.m_range.empty() && this->m_range == other.m_range;
}

bool MeshSet::operator!=(const MeshSet& other) const
{
  return !(*this == other);
}

bool MeshSet::operator<(const MeshSet& other) const
{
  const std::size_t myLen = this->m_range.size();
  const std::size_t otherLen = other.size();

  //only when the number of elements in the two meshsets are equal do
  //we need to do a complex less than comparison
  if (myLen == otherLen)
  {
    //next we look at psize which is the number of pairs inside the range
    const std::size_t myPLen = this->m_range.psize();
    const std::size_t otherPLen = other.m_range.psize();

    if (myPLen == otherPLen)
    {
      //we now have two handle ranges with same number of values, and
      //the same number of pairs. Now we need

      smtk::mesh::HandleRange::const_pair_iterator i, j;
      i = this->m_range.const_pair_begin();
      j = other.m_range.const_pair_begin();
      for (; i != this->m_range.const_pair_end(); ++i, ++j)
      {
        if (i->first != j->first)
        {
          return i->first < j->first;
        }
        else if (i->second != j->second)
        {
          return i->second < j->second;
        }
      }
      //we looped over the entire set and everything was equal, so therefore
      //we must compare parents.

      if (this->m_parent && other.m_parent)
      { //If and only if the two parents exist can we safely compare
        //the uuids of the parents
        return this->m_parent->entity() < other.m_parent->entity();
      }
      else
      {
        //Return true when we have a non null parent
        return !!this->m_parent;
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
  if (!this->m_parent)
  {
    this->m_parent = other.m_parent;
  }
  const bool can_append = this->m_parent == other.m_parent;
  if (can_append)
  {
    this->m_range.insert(other.m_range.begin(), other.m_range.end());
  }
  return can_append;
}

bool MeshSet::is_empty() const
{
  return this->m_range.empty();
}

std::size_t MeshSet::size() const
{
  return this->m_range.size();
}

std::vector<smtk::mesh::Domain> MeshSet::domains() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->computeDomainValues(this->m_range);
}

std::vector<smtk::mesh::Dirichlet> MeshSet::dirichlets() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->computeDirichletValues(this->m_range);
}

std::vector<smtk::mesh::Neumann> MeshSet::neumanns() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->computeNeumannValues(this->m_range);
}

bool MeshSet::setDomain(const smtk::mesh::Domain& d)
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->setDomain(this->m_range, d);
}

bool MeshSet::setDirichlet(const smtk::mesh::Dirichlet& d)
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->setDirichlet(this->m_range, d);
}

bool MeshSet::setNeumann(const smtk::mesh::Neumann& n)
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->setNeumann(this->m_range, n);
}

/**\brief Return an array of model entity UUIDs associated with meshset members.
  *
  */
smtk::common::UUIDArray MeshSet::modelEntityIds() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->computeModelEntities(this->m_range);
}

/**\brief Return the model entities associated with meshset members.
  *
  * warning Note that the parent collection of this meshset must have
  *         its model manager set to a valid value or the result will
  *         be an array of invalid entries.
  */
bool MeshSet::modelEntities(smtk::model::EntityRefArray& array) const
{
  smtk::model::ManagerPtr mgr = this->m_parent->modelManager();
  smtk::common::UUIDArray uids = this->modelEntityIds();
  for (smtk::common::UUIDArray::const_iterator it = uids.begin(); it != uids.end(); ++it)
    array.push_back(smtk::model::EntityRef(mgr, *it));
  return (mgr != nullptr);
}

/**\brief Set the model entity for each meshset member to \a ent.
  *
  */
bool MeshSet::setModelEntity(const smtk::model::EntityRef& ent)
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->setAssociation(ent.entity(), this->m_range);
}

/**\brief Set the model entity for each meshset member to \a ent.
  *
  */
bool MeshSet::setModelEntityId(const smtk::common::UUID& id)
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->setAssociation(id, this->m_range);
}

/**\brief Get the parent collection that this meshset belongs to.
  *
  */
const smtk::mesh::CollectionPtr& MeshSet::collection() const
{
  return this->m_parent;
}

std::vector<std::string> MeshSet::names() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->computeNames(this->m_range);
}

smtk::mesh::TypeSet MeshSet::types() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->computeTypes(this->m_range);
}

smtk::mesh::CellSet MeshSet::cells() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells(this->m_range);
  return smtk::mesh::CellSet(this->m_parent, range);
}

smtk::mesh::PointSet MeshSet::points() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange cells = iface->getCells(this->m_range);
  smtk::mesh::HandleRange range = iface->getPoints(cells);
  return smtk::mesh::PointSet(this->m_parent, range);
}

smtk::mesh::PointConnectivity MeshSet::pointConnectivity() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells(this->m_range);
  return smtk::mesh::PointConnectivity(this->m_parent, range);
}

smtk::mesh::CellSet MeshSet::cells(smtk::mesh::CellType cellType) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells(this->m_range, cellType);
  return smtk::mesh::CellSet(this->m_parent, range);
}

smtk::mesh::CellSet MeshSet::cells(smtk::mesh::CellTypes cellTypes) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells(this->m_range, cellTypes);
  return smtk::mesh::CellSet(this->m_parent, range);
}

smtk::mesh::CellSet MeshSet::cells(smtk::mesh::DimensionType dim) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange range = iface->getCells(this->m_range, dim);
  return smtk::mesh::CellSet(this->m_parent, range);
}

smtk::mesh::MeshSet MeshSet::subset(smtk::mesh::DimensionType dim) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange dimMeshes = iface->getMeshsets(this->m_handle, dim);
  //intersect our mesh id with those of a given dimension to find the subset
  return smtk::mesh::MeshSet(
    this->m_parent, this->m_handle, iface->rangeIntersect(dimMeshes, this->m_range));
}

smtk::mesh::MeshSet MeshSet::subset(const smtk::mesh::Domain& d) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange dMeshes = iface->getMeshsets(this->m_handle, d);
  //intersect our mesh id with those of a given dimension to find the subset
  return smtk::mesh::MeshSet(
    this->m_parent, this->m_handle, iface->rangeIntersect(dMeshes, this->m_range));
}

smtk::mesh::MeshSet MeshSet::subset(const smtk::mesh::Dirichlet& d) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange dMeshes = iface->getMeshsets(this->m_handle, d);
  //intersect our mesh id with those of a given dimension to find the subset
  return smtk::mesh::MeshSet(
    this->m_parent, this->m_handle, iface->rangeIntersect(dMeshes, this->m_range));
}

smtk::mesh::MeshSet MeshSet::subset(const smtk::mesh::Neumann& n) const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  smtk::mesh::HandleRange nMeshes = iface->getMeshsets(this->m_handle, n);
  //intersect our mesh id with those of a given dimension to find the subset
  return smtk::mesh::MeshSet(
    this->m_parent, this->m_handle, iface->rangeIntersect(nMeshes, this->m_range));
}

smtk::mesh::MeshSet MeshSet::subset(std::size_t ith) const
{
  smtk::mesh::HandleRange singlHandleRange;
  if (!this->m_range.empty() && ith < this->m_range.size())
  {
    smtk::mesh::HandleRange::const_iterator cit = this->m_range.begin();
    cit += ith;

    singlHandleRange.insert(*cit);
  }
  smtk::mesh::MeshSet singleMesh(this->m_parent, this->m_handle, singlHandleRange);
  return singleMesh;
}

smtk::mesh::MeshSet MeshSet::extractShell() const
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();

  smtk::mesh::HandleRange entities;
  smtk::mesh::HandleRange cells;
  const bool shellExtracted = iface->computeShell(this->m_range, cells);
  if (shellExtracted)
  {
    smtk::mesh::Handle meshSetHandle;
    //create a mesh for these cells since they don't have a meshset currently
    const bool meshCreated = iface->createMesh(cells, meshSetHandle);
    if (meshCreated)
    {
      entities.insert(meshSetHandle);
    }
  }
  return smtk::mesh::MeshSet(this->m_parent, this->m_handle, entities);
}

bool MeshSet::mergeCoincidentContactPoints(double tolerance)
{
  const smtk::mesh::InterfacePtr& iface = this->m_parent->interface();
  return iface->mergeCoincidentContactPoints(this->m_range, tolerance);
}

smtk::mesh::CellField MeshSet::createCellField(
  const std::string& name, int dimension, const std::vector<double>& data)
{
  assert(data.size() == this->cells().size() * dimension);
  return this->createCellField(name, dimension, &data[0]);
}

smtk::mesh::CellField MeshSet::createCellField(
  const std::string& name, int dimension, const double* const data)
{
  if (name.empty() || dimension <= 0)
  {
    // CellFields must be named and have dimension higher than zero.
    return CellField();
  }

  const smtk::mesh::InterfacePtr& iface = this->collection()->interface();
  if (!iface)
  {
    return CellField();
  }

  bool success;
  if (data != nullptr)
  {
    success = iface->createCellField(m_range, name, dimension, data);
  }
  else
  {
    std::vector<double> tmp(this->cells().size() * dimension, 0.);
    success = iface->createCellField(m_range, name, dimension, &tmp[0]);
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

  const smtk::mesh::InterfacePtr& iface = this->collection()->interface();
  if (!iface)
  {
    return cellfields;
  }

  std::set<smtk::mesh::CellFieldTag> tags = iface->computeCellFieldTags(this->m_handle);
  for (auto& tag : tags)
  {
    if (iface->hasCellField(this->range(), tag))
    {
      cellfields.insert(CellField(*this, tag.name()));
    }
  }

  return cellfields;
}

std::vector<smtk::mesh::CellField> MeshSet::cellFieldsForShiboken() const
{
  auto cellfields = this->cellFields();
  return std::vector<smtk::mesh::CellField>(cellfields.begin(), cellfields.end());
}

bool MeshSet::removeCellField(smtk::mesh::CellField cellfield)
{
  const smtk::mesh::InterfacePtr& iface = this->collection()->interface();

  return iface->deleteCellField(CellFieldTag(cellfield.name()), m_range);
}

smtk::mesh::PointField MeshSet::createPointField(
  const std::string& name, int dimension, const std::vector<double>& data)
{
  assert(data.size() == this->points().size() * dimension);
  return this->createPointField(name, dimension, &data[0]);
}

smtk::mesh::PointField MeshSet::createPointField(
  const std::string& name, int dimension, const double* const data)
{
  if (name.empty() || dimension <= 0)
  {
    // PointFields must be named and have dimension higher than zero.
    return PointField();
  }

  const smtk::mesh::InterfacePtr& iface = this->collection()->interface();
  if (!iface)
  {
    return PointField();
  }

  bool success;
  if (data != nullptr)
  {
    success = iface->createPointField(m_range, name, dimension, data);
  }
  else
  {
    std::vector<double> tmp(this->points().size() * dimension, 0.);
    success = iface->createPointField(m_range, name, dimension, &tmp[0]);
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

  const smtk::mesh::InterfacePtr& iface = this->collection()->interface();
  if (!iface)
  {
    return pointfields;
  }

  std::set<smtk::mesh::PointFieldTag> tags = iface->computePointFieldTags(this->m_handle);
  for (auto& tag : tags)
  {
    if (iface->hasPointField(this->range(), tag))
    {
      pointfields.insert(PointField(*this, tag.name()));
    }
  }

  return pointfields;
}

std::vector<smtk::mesh::PointField> MeshSet::pointFieldsForShiboken() const
{
  auto pointfields = this->pointFields();
  return std::vector<smtk::mesh::PointField>(pointfields.begin(), pointfields.end());
}

bool MeshSet::removePointField(smtk::mesh::PointField pointfield)
{
  const smtk::mesh::InterfacePtr& iface = this->collection()->interface();

  return iface->deletePointField(PointFieldTag(pointfield.name()), m_range);
}

//intersect two mesh sets, placing the results in the return mesh set
MeshSet set_intersect(const MeshSet& a, const MeshSet& b)
{
  if (a.m_parent != b.m_parent)
  { //return an empty MeshSet if the collections don't match
    return smtk::mesh::MeshSet(a.m_parent, a.m_handle, smtk::mesh::HandleRange());
  }

  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();
  smtk::mesh::HandleRange result = iface->rangeIntersect(a.m_range, b.m_range);
  return smtk::mesh::MeshSet(a.m_parent, a.m_handle, result);
}

//subtract mesh b from a, placing the results in the return mesh set
MeshSet set_difference(const MeshSet& a, const MeshSet& b)
{
  if (a.m_parent != b.m_parent)
  { //return an empty MeshSet if the collections don't match
    return smtk::mesh::MeshSet(a.m_parent, a.m_handle, smtk::mesh::HandleRange());
  }

  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();
  smtk::mesh::HandleRange result = iface->rangeDifference(a.m_range, b.m_range);
  return smtk::mesh::MeshSet(a.m_parent, a.m_handle, result);
}

//union two mesh sets, placing the results in the return mesh set
MeshSet set_union(const MeshSet& a, const MeshSet& b)
{
  if (a.m_parent != b.m_parent)
  { //return an empty MeshSet if the collections don't match
    return smtk::mesh::MeshSet(a.m_parent, a.m_handle, smtk::mesh::HandleRange());
  }

  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();
  smtk::mesh::HandleRange result = iface->rangeUnion(a.m_range, b.m_range);
  return smtk::mesh::MeshSet(a.m_parent, a.m_handle, result);
}

SMTKCORE_EXPORT void for_each(const MeshSet& a, MeshForEach& filter)
{
  const smtk::mesh::InterfacePtr& iface = a.m_parent->interface();

  filter.m_collection = a.m_parent;
  iface->meshForEach(a.m_range, filter);
}
}
}
