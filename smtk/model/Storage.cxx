#include "smtk/model/Storage.h"

#include "smtk/model/AttributeAssignments.h"

#include "smtk/model/Chain.h"
#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/Loop.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Shell.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/VertexUse.h"
#include "smtk/model/Volume.h"

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

/// Add an edge to storage (without any relationships)
Vertex Storage::addVertex()
{
  return Vertex(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(CELL_ENTITY, 0));
}

/// Add an edge to storage (without any relationships)
Edge Storage::addEdge()
{
  return Edge(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(CELL_ENTITY, 1));
}

/**\brief Add a face to storage (without any relationships)
  *
  * While this method does not add any relations, it
  * does create two HAS_USE arrangements to hold
  * FaceUse instances (assuming the BRepModel may be
  * downcast to a Storage instance).
  */
Face Storage::addFace()
{
  return Face(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(CELL_ENTITY, 2));
}

/// Add a volume to storage (without any relationships)
Volume Storage::addVolume()
{
  return Volume(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(CELL_ENTITY, 3));
}

/// Add a vertex-use to storage (without any relationships)
VertexUse Storage::addVertexUse()
{
  return VertexUse(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(USE_ENTITY, 0));
}

/// Add an edge-use to storage (without any relationships)
EdgeUse Storage::addEdgeUse()
{
  return EdgeUse(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(USE_ENTITY, 1));
}

/// Add a face-use to storage (without any relationships)
FaceUse Storage::addFaceUse()
{
  return FaceUse(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(USE_ENTITY, 2));
}

/// Add a 0/1-d shell (a vertex chain) to storage (without any relationships)
Chain Storage::addChain()
{
  return Chain(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(SHELL_ENTITY | DIMENSION_0 | DIMENSION_1, -1));
}

/// Add a 1/2-d shell (an edge loop) to storage (without any relationships)
Loop Storage::addLoop()
{
  return Loop(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(SHELL_ENTITY | DIMENSION_1 | DIMENSION_2, -1));
}

/// Add a 2/3-d shell (a face-shell) to storage (without any relationships)
Shell Storage::addShell()
{
  return Shell(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(SHELL_ENTITY | DIMENSION_2 | DIMENSION_3, -1));
}

/**\brief Add an entity group to storage (without any relationships).
  *
  * Any non-zero bits set in \a extraFlags are OR'd with entityFlags() of the group.
  * This is an easy way to constrain the dimension of entities allowed to be members
  * of the group.
  *
  * You may also specify a \a name for the group. If \a name is empty, then no
  * name is assigned.
  */
GroupEntity Storage::addGroup(int extraFlags, const std::string& name)
{
  smtk::util::UUID uid =
    this->addEntityOfTypeAndDimension(GROUP_ENTITY | extraFlags, -1);
  if (!name.empty())
    this->setStringProperty(uid, "name", name);
  return GroupEntity(shared_from_this(), uid);
}

/**\brief Add a model to storage.
  *
  * The model will have the specified \a embeddingDim set as an integer property
  * named "embedding dimension." This is the dimension of the space in which
  * vertex coordinates live.
  *
  * A model may also be given a parametric dimension
  * which is the maximum parametric dimension of any cell inserted into the model.
  * The parametric dimension is the rank of the space spanned by the shape functions
  * (for "parametric" meshes) or (for "discrete" meshes) barycentric coordinates of cells.
  *
  * You may also specify a \a name for the model. If \a name is empty, then no
  * name is assigned.
  *
  * A model maintains counters used to number model entities by type (uniquely within the
  * model). Any entities related to the model (directly or indirectly via topological
  * relationships) may have these numbers assigned as names by calling assignDefaultNames().
  */
ModelEntity Storage::addModel(
  int parametricDim, int embeddingDim, const std::string& name)
{
  smtk::util::UUID uid = this->addEntityOfTypeAndDimension(MODEL_ENTITY, parametricDim);
  if (embeddingDim > 0)
    {
    this->setIntegerProperty(uid, "embedding dimension", embeddingDim);
    }
  if (!name.empty())
    this->setStringProperty(uid, "name", name);
  else
    this->assignDefaultName(uid, this->type(uid));
  return ModelEntity(shared_from_this(), uid);
}


  } // namespace model
} //namespace smtk
