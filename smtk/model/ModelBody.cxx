#include "ModelBody.h"

#include <algorithm>
#include <set>
#include <map>
#include <vector>

#include <sstream>

//#include <boost/variant.hpp>

using namespace std;

namespace smtk {
  namespace model {

ModelBody::ModelBody() :
  BRepModel<UUID,UUIDs,Link>(new UUIDsToLinks, true),
  Relationships(new UUIDsToArrangements),
  Geometry(new UUIDsToTessellations)
{
}

ModelBody::ModelBody(
  UUIDsToLinks* topology,
  UUIDsToArrangements* arrangements,
  UUIDsToTessellations* geometry,
  bool shouldDelete)
  : BRepModel<UUID,UUIDs,Link>(topology, shouldDelete), Relationships(arrangements), Geometry(geometry)
{
}

ModelBody::~ModelBody()
{
  if (this->DeleteStorage)
    {
    delete this->Relationships;
    this->Relationships = NULL;
    delete this->Geometry;
    this->Geometry = NULL;
    }
}

UUIDsToArrangements& ModelBody::arrangements()
{
  return *this->Relationships;
}

const UUIDsToArrangements& ModelBody::arrangements() const
{
  return *this->Relationships;
}

UUIDsToTessellations& ModelBody::tessellations()
{
  return *this->Geometry;
}

const UUIDsToTessellations& ModelBody::tessellations() const
{
  return *this->Geometry;
}


ModelBody::geom_iter_type ModelBody::SetTessellation(const UUID& cellId, const Tessellation& geom)
{
  if (cellId.IsNull())
    {
    throw std::string("Nil cell ID");
    }
  geom_iter_type result = this->Geometry->find(cellId);
  if (result == this->Geometry->end())
    {
    std::pair<UUID,Tessellation> blank;
    blank.first = cellId;
    result = this->Geometry->insert(blank).first;
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
int ModelBody::ArrangeLink(const UUID& cellId, ArrangementKind kind, const Arrangement& arr, int index)
{
  UUIDsToArrangements::iterator cit = this->Relationships->find(cellId);
  if (cit == this->Relationships->end())
    {
    if (index >= 0)
      { // failure: can't replace information that doesn't exist.
      return -1;
      }
    KindsToArrangements blank;
    cit = this->Relationships->insert(std::pair<UUID,KindsToArrangements>(cellId, blank)).first;
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
const Arrangement* ModelBody::GetArrangement(const UUID& cellId, ArrangementKind kind, int index) const
{
  if (cellId.IsNull() || index < 0)
    {
    return NULL;
    }

  UUIDsToArrangements::iterator cit = this->Relationships->find(cellId);
  if (cit == this->Relationships->end())
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
Arrangement* ModelBody::GetArrangement(const UUID& cellId, ArrangementKind kind, int index)
{
  if (cellId.IsNull() || index < 0)
    {
    return NULL;
    }

  UUIDsToArrangements::iterator cit = this->Relationships->find(cellId);
  if (cit == this->Relationships->end())
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
