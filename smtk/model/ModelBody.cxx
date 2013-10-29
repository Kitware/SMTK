#include "ModelBody.h"

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

ModelBody::ModelBody() :
  BRepModel(new UUIDsToLinks, true),
  m_relationships(new UUIDsToArrangements),
  m_geometry(new UUIDsToTessellations)
{
}

ModelBody::ModelBody(
  UUIDsToLinks* topology,
  UUIDsToArrangements* arrangements,
  UUIDsToTessellations* geometry,
  bool shouldDelete)
  : BRepModel(topology, shouldDelete), m_relationships(arrangements), m_geometry(geometry)
{
}

ModelBody::~ModelBody()
{
  if (this->m_deleteStorage)
    {
    delete this->m_relationships;
    this->m_relationships = NULL;
    delete this->m_geometry;
    this->m_geometry = NULL;
    }
}

UUIDsToArrangements& ModelBody::arrangements()
{
  return *this->m_relationships;
}

const UUIDsToArrangements& ModelBody::arrangements() const
{
  return *this->m_relationships;
}

UUIDsToTessellations& ModelBody::tessellations()
{
  return *this->m_geometry;
}

const UUIDsToTessellations& ModelBody::tessellations() const
{
  return *this->m_geometry;
}


ModelBody::tess_iter_type ModelBody::setTessellation(const UUID& cellId, const Tessellation& geom)
{
  if (cellId.IsNull())
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
int ModelBody::arrangeLink(const UUID& cellId, ArrangementKind kind, const Arrangement& arr, int index)
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

/**\brief Retrieve arrangement information for a cell.
  *
  * This version does not allow the arrangement to be altered.
  */
const Arrangement* ModelBody::findArrangement(const UUID& cellId, ArrangementKind kind, int index) const
{
  if (cellId.IsNull() || index < 0)
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
Arrangement* ModelBody::findArrangement(const UUID& cellId, ArrangementKind kind, int index)
{
  if (cellId.IsNull() || index < 0)
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
