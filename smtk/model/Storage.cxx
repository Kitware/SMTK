#include "smtk/model/Storage.h"

#include "smtk/model/AttributeAssignments.h"

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
  BRepModel(shared_ptr<UUIDsToEntities>(new UUIDsToEntities)),
  m_arrangements(new UUIDsToArrangements),
  m_tessellations(new UUIDsToTessellations),
  m_attributeAssignments(new UUIDsToAttributeAssignments)
{
}

Storage::Storage(
  shared_ptr<UUIDsToEntities> inTopology,
  shared_ptr<UUIDsToArrangements> inArrangements,
  shared_ptr<UUIDsToTessellations> tess,
  shared_ptr<UUIDsToAttributeAssignments> attribs)
  :
    BRepModel(inTopology), m_arrangements(inArrangements),
    m_tessellations(tess), m_attributeAssignments(attribs)
{
}

Storage::~Storage()
{
}

UUIDsToArrangements& Storage::arrangements()
{
  return *this->m_arrangements.get();
}

const UUIDsToArrangements& Storage::arrangements() const
{
  return *this->m_arrangements.get();
}

UUIDsToTessellations& Storage::tessellations()
{
  return *this->m_tessellations.get();
}

const UUIDsToTessellations& Storage::tessellations() const
{
  return *this->m_tessellations.get();
}

UUIDsToAttributeAssignments& Storage::attributeAssignments()
{
  return *this->m_attributeAssignments;
}

const UUIDsToAttributeAssignments& Storage::attributeAssignments() const
{
  return *this->m_attributeAssignments;
}

Storage::tess_iter_type Storage::setTessellation(const UUID& cellId, const Tessellation& geom)
{
  if (cellId.isNull())
    {
    throw std::string("Nil cell ID");
    }
  tess_iter_type result = this->m_tessellations->find(cellId);
  if (result == this->m_tessellations->end())
    {
    std::pair<UUID,Tessellation> blank;
    blank.first = cellId;
    result = this->m_tessellations->insert(blank).first;
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
  UUIDsToArrangements::iterator cit = this->m_arrangements->find(cellId);
  if (cit == this->m_arrangements->end())
    {
    if (index >= 0)
      { // failure: can't replace information that doesn't exist.
      return -1;
      }
    KindsToArrangements blank;
    cit = this->m_arrangements->insert(std::pair<UUID,KindsToArrangements>(cellId, blank)).first;
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
Arrangements* Storage::hasArrangementsOfKindForEntity(
  const smtk::util::UUID& entity, ArrangementKind kind)
{
  UUIDWithArrangementDictionary cellEntry = this->m_arrangements->find(entity);
  if (cellEntry != this->m_arrangements->end())
    {
    ArrangementKindWithArrangements useIt = cellEntry->second.find(kind);
    if (useIt != cellEntry->second.end())
      {
      return &useIt->second;
      }
    }
  return NULL;
}

/**\brief This is a const version of hasArrangementsOfKindForEntity().
  */
const Arrangements* Storage::hasArrangementsOfKindForEntity(
  const smtk::util::UUID& entity, ArrangementKind kind) const
{
  UUIDWithArrangementDictionary cellEntry = this->m_arrangements->find(entity);
  if (cellEntry != this->m_arrangements->end())
    {
    ArrangementKindWithArrangements useIt = cellEntry->second.find(kind);
    if (useIt != cellEntry->second.end())
      {
      return &useIt->second;
      }
    }
  return NULL;
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
  return (*this->m_arrangements)[entity][kind];
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

  UUIDsToArrangements::iterator cit = this->m_arrangements->find(cellId);
  if (cit == this->m_arrangements->end())
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

  UUIDsToArrangements::iterator cit = this->m_arrangements->find(cellId);
  if (cit == this->m_arrangements->end())
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

/**\brief Return the UUID of a use record for the
  * given \a cell and \a sense, or NULL if it does not exist.
  */
smtk::util::UUID Storage::cellHasUseOfSense(
  const smtk::util::UUID& cell, int sense) const
{
  const smtk::model::Arrangements* arr;
  if ((arr = this->hasArrangementsOfKindForEntity(cell, HAS_USE)) && !arr->empty())
    { // See if any of this cell's uses match our sense.
    for (smtk::model::Arrangements::const_iterator ait = arr->begin(); ait != arr->end(); ++ait)
      {
      if (ait->details()[1] == sense)
        {
        return this->findEntity(cell)->relations()[ait->details()[0]];
        }
      }
    }
  return smtk::util::UUID::null();
}

/**\brief Find a use record for the given \a cell and \a sense,
  * creating one if it does not exist.
  */
smtk::util::UUID Storage::findOrCreateCellUseOfSense(
  const smtk::util::UUID& cell, int sense)
{
  Entity* entity = this->findEntity(cell);
  if (!entity)
    {
    return smtk::util::UUID::null();
    }
  smtk::model::Arrangements& arr(
    this->arrangementsOfKindForEntity(cell, HAS_USE));

  // See if any of this cell's uses match our sense...
  smtk::model::Arrangements::const_iterator ait;
  for (ait = arr.begin(); ait != arr.end(); ++ait)
    {
    if (ait->details()[1] == sense)
      {
      return entity->relations()[ait->details()[0]];
      }
    }

  // ...nope, we need to create a new use with
  // the specified sense relative to the cell.
  UUIDWithEntity use = this->insertEntityOfTypeAndDimension(
    USE_ENTITY | entity->dimensionBits(), entity->dimension());
  // We must re-fetch entity since inserting the use
  // may have invalidated our reference to it.
  entity = this->findEntity(cell);

  // Now add the use to the cell and the cell to the use:
  smtk::util::UUIDArray::size_type useIdx = entity->relations().size();
  entity->appendRelation(use->first);
  smtk::util::UUIDArray::size_type cellIdx = use->second.relations().size();
  use->second.appendRelation(cell);

  this->arrangeEntity(
    cell, HAS_USE,
    Arrangement::CellHasUseWithIndexAndSense(static_cast<int>(useIdx), sense));
  this->arrangeEntity(
    use->first, HAS_CELL,
    Arrangement::UseHasCellWithIndexAndSense(static_cast<int>(cellIdx), sense));

  return use->first;
}

/**\brief Report whether an entity has been assigned an attribute.
  *
  */
bool Storage::hasAttribute(int attribId, const smtk::util::UUID& toEntity)
{
  UUIDWithAttributeAssignments it = this->m_attributeAssignments->find(toEntity);
  if (it == this->m_attributeAssignments->end())
    {
    return false;
    }
  return it->second.isAssociated(attribId);
}

/**\brief Assign an attribute to an entity.
  *
  */
bool Storage::attachAttribute(int attribId, const smtk::util::UUID& toEntity)
{
  return this->attributeAssignments()[toEntity].attachAttribute(attribId);
}

/**\brief Unassign an attribute from an entity.
  *
  */
bool Storage::detachAttribute(int attribId, const smtk::util::UUID& fromEntity, bool reverse)
{
  bool didRemove = false;
  UUIDWithAttributeAssignments ref = this->m_attributeAssignments->find(fromEntity);
  if (ref == this->m_attributeAssignments->end())
    {
    return didRemove;
    }
  if ((didRemove = ref->second.detachAttribute(attribId)) && reverse)
    {
    /* FIXME: Let manager know.
    smtk::attribute::Manager* mgr = smtk::attribute::Manager::getGlobalManager();
    if (mgr)
      {
      smtk::attribute::AttributePtr attrib = mgr->findAttribute(attribId);
      if (attrib)
        {
        // We don't need a shared pointer to ourselves since we are
        // passing reverse=false here. Thus we can get by without
        // inheriting shared_from_this(), which is a rat's nest
        // should anyone ever derive a class from Storage.
        smtk::model::StoragePtr model; // a NULL shared pointer.
        attrib->disassociateEntity(model, fromEntity, false);
        }
      }
      */
    }
  return didRemove;
}

  } // namespace model
} //namespace smtk
