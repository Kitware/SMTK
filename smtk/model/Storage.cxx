#include "smtk/model/Storage.h"

#include <algorithm>
#include <set>
#include <map>
#include <vector>

#include <sstream>

//#include <boost/variant.hpp>

using namespace std;
using namespace smtk::util;

namespace smtk {
  namespace model {

Storage::Storage() :
  BRepModel(new UUIDsToEntities, true),
  m_relationships(new UUIDsToArrangements),
  m_geometry(new UUIDsToTessellations)
{
}

Storage::Storage(
  UUIDsToEntities* topology,
  UUIDsToArrangements* arrangements,
  UUIDsToTessellations* geometry,
  bool shouldDelete)
  : BRepModel(topology, shouldDelete), m_relationships(arrangements), m_geometry(geometry)
{
}

Storage::~Storage()
{
  if (this->m_deleteStorage)
    {
    delete this->m_relationships;
    this->m_relationships = NULL;
    delete this->m_geometry;
    this->m_geometry = NULL;
    }
}

UUIDsToArrangements& Storage::arrangements()
{
  return *this->m_relationships;
}

const UUIDsToArrangements& Storage::arrangements() const
{
  return *this->m_relationships;
}

UUIDsToTessellations& Storage::tessellations()
{
  return *this->m_geometry;
}

const UUIDsToTessellations& Storage::tessellations() const
{
  return *this->m_geometry;
}


Storage::tess_iter_type Storage::setTessellation(const UUID& cellId, const Tessellation& geom)
{
  if (cellId.isNull())
    {
    throw std::string("Nil cell ID");
    }
  tess_iter_type result = this->m_geometry->find(cellId);
  if (result == this->m_geometry->end())
    {
    std::pair<UUID,Tessellation> blank;
    blank.first = cellId;
    result = this->m_geometry->insert(blank).first;
    }
  result->second = geom;
  return result;
}

/**\brief Add or replace information about the arrangement of a cell.
  *
  * When \a index is -1, the arrangement is considered new and added to the end of
  * the vector of arrangements of the given \a kind.
  * Otherwise, it should be positive and refer to a pre-existing arrangement to be replaced.
  * The actual \a index location used is returned.
  */
int Storage::arrangeEntity(const UUID& cellId, ArrangementKind kind, const Arrangement& arr, int index)
{
  UUIDsToArrangements::iterator cit = this->m_relationships->find(cellId);
  if (cit == this->m_relationships->end())
    {
    if (index >= 0)
      { // failure: can't replace information that doesn't exist.
      return -1;
      }
    KindsToArrangements blank;
    cit = this->m_relationships->insert(std::pair<UUID,KindsToArrangements>(cellId, blank)).first;
    }
  KindsToArrangements::iterator kit = cit->second.find(kind);
  if (kit == cit->second.end())
    {
    if (index >= 0)
      { // failure: can't replace information that doesn't exist.
      return -1;
      }
    Arrangements blank;
    kit = cit->second.insert(std::pair<ArrangementKind,Arrangements>(kind, blank)).first;
    }

  if (index >= 0)
    {
    if (index >= static_cast<int>(kit->second.size()))
      { // failure: can't replace information that doesn't exist.
      return -1;
      }
    kit->second[index] = arr;
    }
  else
    {
    index = static_cast<int>(kit->second.size());
    kit->second.push_back(arr);
    }
  return index;
}

/**\brief Returns true when the given \a entity has any arrangements of the given \a kind (otherwise false).
  *
  * Use this to avoid accidentally inserting a new array of arrangements with arrangementsOfKindForEntity().
  * Since this actually requires a lookup, you may pass in a pointer \a arr to an array of arrangements;
  * if true is returned, the pointer will be aimed at the existing array. Otherwise, \a arr will be unchanged.
  */
bool Storage::hasArrangementsOfKindForEntity(
  const smtk::util::UUID& entity, ArrangementKind kind, Arrangements* arr)
{
  UUIDWithArrangementDictionary cellEntry = this->m_relationships->find(entity);
  if (cellEntry != this->m_relationships->end())
    {
    ArrangementKindWithArrangements useIt = cellEntry->second.find(kind);
    if (useIt != cellEntry->second.end())
      {
      if (arr)
        {
        arr = &useIt->second;
        }
      return true;
      }
    }
  return false;
}

/**\brief This is a const version of hasArrangementsOfKindForEntity().
  */
bool Storage::hasArrangementsOfKindForEntity(
  const smtk::util::UUID& entity, ArrangementKind kind, Arrangements const* arr) const
{
  UUIDWithArrangementDictionary cellEntry = this->m_relationships->find(entity);
  if (cellEntry != this->m_relationships->end())
    {
    ArrangementKindWithArrangements useIt = cellEntry->second.find(kind);
    if (useIt != cellEntry->second.end())
      {
      if (arr)
        {
        arr = &useIt->second;
        }
      return true;
      }
    }
  return false;
}

/**\brief Return an array of arrangements of the given \a kind for the given \a entity.
  *
  * NOTE: This method will create an empty array and attach it to the entity
  * if none exists, thus increasing storage costs. Unless you intend to append
  * new relationships, you should not use this method without first calling
  * hasArrangementsOfKindForEntity() to determine whether the array already exists.
  */
Arrangements& Storage::arrangementsOfKindForEntity(
  const smtk::util::UUID& entity,
  ArrangementKind kind)
{
  return (*this->m_relationships)[entity][kind];
}

/**\brief Retrieve arrangement information for a cell.
  *
  * This version does not allow the arrangement to be altered.
  */
const Arrangement* Storage::findArrangement(const UUID& cellId, ArrangementKind kind, int index) const
{
  if (cellId.isNull() || index < 0)
    {
    return NULL;
    }

  UUIDsToArrangements::iterator cit = this->m_relationships->find(cellId);
  if (cit == this->m_relationships->end())
    {
    return NULL;
    }

  KindsToArrangements::iterator kit = cit->second.find(kind);
  if (kit == cit->second.end())
    {
    return NULL;
    }

  if (index >= static_cast<int>(kit->second.size()))
    { // failure: can't replace information that doesn't exist.
    return NULL;
    }
  return &kit->second[index];
}

/**\brief Retrieve arrangement information for a cell.
  *
  * This version allows the arrangement to be altered.
  */
Arrangement* Storage::findArrangement(const UUID& cellId, ArrangementKind kind, int index)
{
  if (cellId.isNull() || index < 0)
    {
    return NULL;
    }

  UUIDsToArrangements::iterator cit = this->m_relationships->find(cellId);
  if (cit == this->m_relationships->end())
    {
    return NULL;
    }

  KindsToArrangements::iterator kit = cit->second.find(kind);
  if (kit == cit->second.end())
    {
    return NULL;
    }

  if (index >= static_cast<int>(kit->second.size()))
    { // failure: can't replace information that doesn't exist.
    return NULL;
    }
  return &kit->second[index];
}

  } // namespace model
} //namespace smtk
