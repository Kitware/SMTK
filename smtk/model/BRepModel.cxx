#include "smtk/model/BRepModel.h"

#include "smtk/model/Cursor.h"
#include "smtk/model/CursorArrangementOps.h"
#include "smtk/model/DefaultBridge.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Storage.h"

namespace smtk {
  namespace model {

using smtk::util::UUID;
using smtk::util::UUIDs;
using smtk::util::UUIDArray;

/**\brief Construction requires a container for storage.
  *
  * Storage is kept separate so that it can easily be serialized and deserialized.
  */
BRepModel::BRepModel() :
  m_topology(new UUIDsToEntities),
  m_floatData(new UUIDsToFloatData),
  m_stringData(new UUIDsToStringData),
  m_integerData(new UUIDsToIntegerData),
  m_defaultBridge(DefaultBridge::create()),
  m_modelCount(1)
{
  // TODO: throw() when topology == NULL?
}

/**\brief Construction requires a container for storage.
  *
  * Storage is kept separate so that it can easily be serialized and deserialized.
  */
BRepModel::BRepModel(shared_ptr<UUIDsToEntities> topo) :
  m_topology(topo),
  m_floatData(new UUIDsToFloatData),
  m_stringData(new UUIDsToStringData),
  m_integerData(new UUIDsToIntegerData),
  m_defaultBridge(DefaultBridge::create()),
  m_modelCount(1)
    { } // TODO: throw() when topology == NULL?

BRepModel::~BRepModel()
{
}

UUIDsToEntities& BRepModel::topology()
{
  return *this->m_topology.get();
}

const UUIDsToEntities& BRepModel::topology() const
{
  return *this->m_topology.get();
}

/// Entity construction
//@{
/// Return a currently-unused UUID (guaranteed not to collide if inserted immediately).
smtk::util::UUID BRepModel::unusedUUID()
{
  UUID actual;
  do
    {
    actual = this->m_uuidGenerator.random();
    }
  while (this->m_topology->find(actual) != this->m_topology->end());
  return actual;
}

/// Insert a new cell of the specified \a dimension, returning an iterator with a new, unique UUID.
BRepModel::iter_type BRepModel::insertEntityOfTypeAndDimension(BitFlags entityFlags, int dim)
{
  UUID actual = this->unusedUUID();
  return this->setEntityOfTypeAndDimension(actual, entityFlags, dim);
}

/// Insert the specified cell, returning an iterator with a new, unique UUID.
BRepModel::iter_type BRepModel::insertEntity(Entity& c)
{
  UUID actual = this->unusedUUID();
  return this->setEntity(actual, c);
}

/**\brief Create and map a new cell of the given \a dimension to the given \a uid.
  *
  * Passing a null or non-unique \a uid is an error here and will throw an exception.
  *
  * Some checking and initialization is performed based on \a entityFlags and \a dim,
  * as described below.
  *
  * If the BRepModel may be cast to a Storage instance and an entity
  * is expected to have a known, fixed number of arrangements of some sort,
  * those are created here so that cursors may always rely on their existence
  * even in the absence of the related UUIDs appearing in the entity's relations.
  * For face cells (CELL_2D) entites, two HAS_USE Arrangements are created to
  * reference FaceUse instances.
  */
BRepModel::iter_type BRepModel::setEntityOfTypeAndDimension(const UUID& uid, BitFlags entityFlags, int dim)
{
  UUIDsToEntities::iterator it;
  if (uid.isNull())
    {
    std::ostringstream msg;
    msg << "Nil UUID";
    throw msg.str();
    }
  if ((it = this->m_topology->find(uid)) != this->m_topology->end())
    {
    std::ostringstream msg;
    msg << "Duplicate UUID '" << uid << "' of different dimension " << it->second.dimension() << " != " << dim;
    throw msg.str();
    }
  std::pair<UUID,Entity> entry(uid,Entity(entityFlags, dim));
  this->prepareForEntity(entry);
  std::pair<BRepModel::iter_type,bool> result = this->m_topology->insert(entry);

  if (result.second)
    {
    Storage* store = dynamic_cast<Storage*>(this);
    if (store)
      store->trigger(std::make_pair(ADD_EVENT, ENTITY_ENTRY),
        Cursor(store->shared_from_this(), uid));
    }

  return result.first;
}

/**\brief Map the specified cell \a c to the given \a uid.
  *
  * Passing a nil or non-unique \a uid is an error here and will throw an exception.
  */
BRepModel::iter_type BRepModel::setEntity(const UUID& uid, Entity& c)
{
  UUIDsToEntities::iterator it;
  if (uid.isNull())
    {
    std::ostringstream msg;
    msg << "Nil UUID";
    throw msg.str();
    }
  if ((it = this->m_topology->find(uid)) != this->m_topology->end())
    {
    if (it->second.dimension() != c.dimension())
      {
      std::ostringstream msg;
      msg << "Duplicate UUID '" << uid << "' of different dimension " << it->second.dimension() << " != " << c.dimension();
      throw msg.str();
      }
    this->removeEntityReferences(it);
    it->second = c;
    this->insertEntityReferences(it);
    return it;
    }
  std::pair<UUID,Entity> entry(uid,c);
  this->prepareForEntity(entry);
  it = this->m_topology->insert(entry).first;
  this->insertEntityReferences(it);
  return it;
}

/// A wrappable version of InsertEntityOfTypeAndDimension
UUID BRepModel::addEntityOfTypeAndDimension(BitFlags entityFlags, int dim)
{
  return this->insertEntityOfTypeAndDimension(entityFlags, dim)->first;
}

/// A wrappable version of InsertEntity
UUID BRepModel::addEntity(Entity& cell)
{
  return this->insertEntity(cell)->first;
}

/// A wrappable version of SetEntityOfTypeAndDimension
UUID BRepModel::addEntityOfTypeAndDimensionWithUUID(const UUID& uid, BitFlags entityFlags, int dim)
{
  return this->setEntityOfTypeAndDimension(uid, entityFlags, dim)->first;
}

/// A wrappable version of SetEntity
UUID BRepModel::addEntityWithUUID(const UUID& uid, Entity& cell)
{
  return this->setEntity(uid, cell)->first;
}
//@}

/// Shortcuts for inserting cells with default entity flags.
//@{
BRepModel::iter_type BRepModel::insertCellOfDimension(int dim)
{
  return this->insertEntityOfTypeAndDimension(CELL_ENTITY, dim);
}

BRepModel::iter_type BRepModel::setCellOfDimension(const UUID& uid, int dim)
{
  return this->setEntityOfTypeAndDimension(uid, CELL_ENTITY, dim);
}

UUID BRepModel::addCellOfDimension(int dim)
{
  return this->addEntityOfTypeAndDimension(CELL_ENTITY, dim);
}

UUID BRepModel::addCellOfDimensionWithUUID(const UUID& uid, int dim)
{
  return this->addEntityOfTypeAndDimensionWithUUID(uid, CELL_ENTITY, dim);
}
//@}

/// Queries on entities belonging to the solid.
//@{
/// Return the type of entity that the link represents.
BitFlags BRepModel::type(const smtk::util::UUID& ofEntity) const
{
  UUIDsToEntities::iterator it = this->m_topology->find(ofEntity);
  return (it == this->m_topology->end() ? INVALID : it->second.entityFlags());
}

/// Return the dimension of the manifold that the passed entity represents.
int BRepModel::dimension(const UUID& ofEntity) const
{
  UUIDsToEntities::iterator it = this->m_topology->find(ofEntity);
  return (it == this->m_topology->end() ? -1 : it->second.dimension());
}

/**\brief Return a name for the given entity ID.
  *
  * This will either return a user-specified name or the "short UUID" name
  * of the entity. It will not assign a name to the entity using the model
  * counters because the method is const.
  */
std::string BRepModel::name(const UUID& ofEntity) const
{
  if (this->hasStringProperty(ofEntity, "name"))
    {
    smtk::model::StringList const& nprop(this->stringProperty(ofEntity, "name"));
    if (!nprop.empty())
      {
      return nprop[0];
      }
    }
  UUIDsToEntities::iterator it = this->m_topology->find(ofEntity);
  if (it == this->m_topology->end())
    {
    return "invalid id " + ofEntity.toString();
    }
  return BRepModel::shortUUIDName(it->first, it->second.entityFlags());
}

/**\brief Return the (Dimension+1 or higher)-entities that are the immediate bordants of the passed entity.
  *
  * \sa HigherDimensionalBoundaries
  */
UUIDs BRepModel::bordantEntities(const smtk::util::UUID& ofEntity, int ofDimension)
{
  UUIDs result;
  UUIDsToEntities::iterator it = this->m_topology->find(ofEntity);
  if (it == this->m_topology->end())
    {
    return result;
    }
  if (ofDimension >= 0 && it->second.dimension() >= ofDimension)
    {
    // can't ask for "higher" dimensional boundaries that are lower than the dimension of this cell.
    return result;
    }
  UUIDsToEntities::iterator other;
  for (UUIDArray::iterator ai = it->second.relations().begin(); ai != it->second.relations().end(); ++ai)
    {
    other = this->m_topology->find(*ai);
    if (other == this->m_topology->end())
      { // TODO: silently skip bad relations or complain?
      continue;
      }
    if (
      (ofDimension >= 0 && other->second.dimension() == ofDimension) ||
      (ofDimension == -2 && other->second.dimension() >= it->second.dimension()))
      {
      result.insert(*ai);
      }
    }
  return result;
}

/**\brief Return the (Dimension+1 or higher)-entities that are the immediate bordants of any of the passed entities.
  *
  * \sa HigherDimensionalBoundaries
  */
UUIDs BRepModel::bordantEntities(const smtk::util::UUIDs& ofEntities, int ofDimension)
{
  UUIDs result;
  std::insert_iterator<UUIDs> inserter(result, result.begin());
  for (UUIDs::const_iterator it = ofEntities.begin(); it != ofEntities.end(); ++it)
    {
    UUIDs bdy = this->bordantEntities(*it, ofDimension);
    std::copy(bdy.begin(), bdy.end(), inserter);
    }
  return result;
}

/**\brief Return the (Dimension-1 or lower)-entities that are the immediate boundary of the passed entity.
  *
  * \sa LowerDimensionalBoundaries
  */
UUIDs BRepModel::boundaryEntities(const smtk::util::UUID& ofEntity, int ofDimension)
{
  UUIDs result;
  UUIDsToEntities::iterator it = this->m_topology->find(ofEntity);
  if (it == this->m_topology->end())
    {
    return result;
    }
  if (ofDimension >= 0 && it->second.dimension() <= ofDimension)
    {
    // can't ask for "lower" dimensional boundaries that are higher than the dimension of this cell.
    return result;
    }
  UUIDsToEntities::iterator other;
  for (UUIDArray::iterator ai = it->second.relations().begin(); ai != it->second.relations().end(); ++ai)
    {
    other = this->m_topology->find(*ai);
    if (other == this->m_topology->end())
      { // TODO: silently skip bad relations or complain?
      continue;
      }
    if (
      (ofDimension >= 0 && other->second.dimension() == ofDimension) ||
      (ofDimension == -2 && other->second.dimension() <= it->second.dimension()))
      {
      result.insert(*ai);
      }
    }
  return result;
}

/**\brief Return the (Dimension-1 or lower)-entities that are the immediate boundary of any of the passed entities.
  *
  * \sa LowerDimensionalBoundaries
  */
UUIDs BRepModel::boundaryEntities(const smtk::util::UUIDs& ofEntities, int ofDimension)
{
  UUIDs result;
  std::insert_iterator<UUIDs> inserter(result, result.begin());
  for (UUIDs::const_iterator it = ofEntities.begin(); it != ofEntities.end(); ++it)
    {
    UUIDs bdy = this->boundaryEntities(*it, ofDimension);
    std::copy(bdy.begin(), bdy.end(), inserter);
    }
  return result;
}

/**\brief Return lower-dimensional boundaries of the passed d-dimensional entity.
  *
  * \a lowerDimension may be any dimension < d.
  * Unlike BRepModel::boundaryEntities(), this method will search the boundaries
  * of the entity's boundaries.
  * For example, a 2-dimensional face normally stores 1-dimensional edges
  * as its immediate boundaries, so if BoundaryEntities() is asked for 0-dimensional
  * entities none will usually be reported (the exception being an isolated vertex
  * lying on the face with no edges attached).
  * But LowerDimensionalBoundaries() will return the corners of the edges when asked
  * for 0-dimensional boundaries.
  *
  * Passing -1 will return all boundary entities of the specified entity,
  * regardless of their dimension.
  */
UUIDs BRepModel::lowerDimensionalBoundaries(const UUID& ofEntity, int lowerDimension)
{
  UUIDs result;
  UUIDsToEntities::iterator it = this->m_topology->find(ofEntity);
  if (it == this->m_topology->end())
    {
    return result;
    }
  if (it->second.dimension() <= lowerDimension)
    {
    // do nothing
    }
  else
    {
    // FIXME: This only works for the "usual" case where
    //        a cell's relations are dimension (d+1) or
    //        (d-1). We should also collect any out-of-place
    //        relations that match lowerDimension as we go.
    int currentDim = it->second.dimension() - 1;
    int delta = currentDim - lowerDimension;
    result = this->boundaryEntities(ofEntity, currentDim--);
    for (int i = delta; i > 0; --i, --currentDim)
      {
      result = this->boundaryEntities(result, currentDim);
      }
    }
  return result;
}

/**\brief Return higher-dimensional bordants of the passed d-dimensional entity.
  *
  * \a higherDimension may be any dimension > d.
  * Unlike BRepModel::bordantEntities(), this method will search the bordants
  * of the entity's immediate bordants.
  * For example, a 1-dimensional edge normally stores 2-dimensional faces
  * as its immediate bordants, so if BoundaryEntities() is asked for 3-dimensional
  * bordants none will usually be reported (the exception being when the edge
  * is contained completely inside the volume and not attached to any boundary).
  * But HigherDimensionalBoundaries() will return all the volumes the edge borders
  * when asked for 3-dimensional boundaries.
  *
  * Passing -1 will return all bordant entities of the specified entity,
  * regardless of their dimension.
  */
UUIDs BRepModel::higherDimensionalBordants(const UUID& ofEntity, int higherDimension)
{
  UUIDs result;
  UUIDsToEntities::iterator it = this->m_topology->find(ofEntity);
  if (it == this->m_topology->end())
    {
    return result;
    }
  if (it->second.dimension() >= higherDimension)
    {
    // do nothing
    }
  else
    {
    int currentDim = it->second.dimension() + 1;
    int delta = higherDimension - currentDim;
    result = this->bordantEntities(ofEntity, currentDim++);
    for (int i = delta; i > 0; --i, ++currentDim)
      {
      result = this->bordantEntities(result, currentDim);
      }
    }
  return result;
}

/// Return entities of the requested dimension that share a boundary relationship with the passed entity.
UUIDs BRepModel::adjacentEntities(const UUID& ofEntity, int ofDimension)
{
  // FIXME: Implement adjacency
  (void)ofEntity;
  (void)ofDimension;
  UUIDs result;
  return result;
}

/// Return all entities of the requested dimension that are present in the solid.
UUIDs BRepModel::entitiesMatchingFlags(BitFlags mask, bool exactMatch)
{
  UUIDs result;
  for (UUIDWithEntity it = this->m_topology->begin(); it != this->m_topology->end(); ++it)
    {
    BitFlags masked = it->second.entityFlags() & mask;
    if (
      (!exactMatch && masked) ||
      (exactMatch && masked == mask))
      {
      result.insert(it->first);
      }
    }
  return result;
}

/// Return all entities of the requested dimension that are present in the solid.
UUIDs BRepModel::entitiesOfDimension(int dim)
{
  UUIDs result;
  for (UUIDWithEntity it = this->m_topology->begin(); it != this->m_topology->end(); ++it)
    {
    if (it->second.dimension() == dim)
      {
      result.insert(it->first);
      }
    }
  return result;
}
//@}

/// Return the smtk::model::Entity associated with \a uid (or NULL).
//@{
const Entity* BRepModel::findEntity(const UUID& uid) const
{
  UUIDWithEntity it = this->m_topology->find(uid);
  if (it == this->m_topology->end())
    {
    return NULL;
    }
  return &it->second;
}

Entity* BRepModel::findEntity(const UUID& uid)
{
  UUIDWithEntity it = this->m_topology->find(uid);
  if (it == this->m_topology->end())
    {
    return NULL;
    }
  return &it->second;
}
//@}

/**\brief Remove the entity with the given \a uid.
  *
  * Returns true upon success, false when the entity did not exist.
  */
bool BRepModel::erase(const smtk::util::UUID& uid)
{
  UUIDWithEntity ent = this->m_topology->find(uid);
  if (ent == this->m_topology->end())
    return false;

  bool isModel = isModelEntity(ent->second.entityFlags());

  Storage* store = dynamic_cast<Storage*>(this);
  if (store)
    store->trigger(std::make_pair(DEL_EVENT, ENTITY_ENTRY),
      Cursor(store->shared_from_this(), uid));

  // TODO: If this entity is a model and has an entry in m_modelBridges,
  //       we should verify that any submodels retain a reference to the
  //       Bridge in m_modelBridges.

  // Before removing the entity, loop through its relations and
  // make sure none of them retain any references back to \a uid.
  // However, we cannot erase entries in relatedEntity->relations()
  // because relatedEntity's arrangements reference them by integer
  // index. Thus, we call elideEntityReferences rather than removeEntityReferences.
  this->elideEntityReferences(ent);

  // TODO: Notify model of entity removal?
  this->m_topology->erase(ent);

  // If the entity was a model, remove any bridge entry for it.
  if (isModel)
    this->m_modelBridges.erase(uid);

  return true;
}

/// Given an entity \a c, ensure that all of its references contain a reference to it.
void BRepModel::insertEntityReferences(const UUIDWithEntity& c)
{
  UUIDArray::const_iterator bit;
  Entity* ref;
  for (bit = c->second.relations().begin(); bit != c->second.relations().end(); ++bit)
    {
    ref = this->findEntity(*bit);
    if (ref)
      {
      ref->appendRelation(c->first);
      }
    }
}

/**\brief Given an entity \a c, ensure that all of its references
  *       contain <b>no</b> reference to it.
  *
  * This is accomplished by overwriting matching references with
  * UUID::null() rather than removing the reference from the array.
  * We do things this way because indices into the list of
  * relations are used by arrangements and we do not want to
  * rewrite arrangements.
  */
void BRepModel::elideEntityReferences(const UUIDWithEntity& c)
{
  UUIDArray::const_iterator bit;
  Entity* ref;
  for (bit = c->second.relations().begin(); bit != c->second.relations().end(); ++bit)
    {
    ref = this->findEntity(*bit);
    if (ref)
      {
      UUIDArray::iterator rit;
      for (rit = ref->relations().begin(); rit != ref->relations().end(); ++rit)
        {
        if (*rit == c->first)
          { // TODO: Notify *bit of imminent elision?
          *rit = smtk::util::UUID::null();
          }
        }
      }
    }
}

/// Given an entity \a c, ensure that all of its references contain <b>no</b> reference to it.
void BRepModel::removeEntityReferences(const UUIDWithEntity& c)
{
  UUIDArray::const_iterator bit;
  Entity* ref;
  for (bit = c->second.relations().begin(); bit != c->second.relations().end(); ++bit)
    {
    ref = this->findEntity(*bit);
    if (ref)
      {
      ref->removeRelation(c->first);
      }
    }
}

/**\brief Add entities (specified by their \a uids) to the given group (\a groupId).
  *
  * This will append \a groupId to each entity in \a uids.
  * Note that this does **not** add the proper Arrangement information
  * that Storage::findOrAddEntityToGroup() does, since BRepModel
  * does not store Arrangement information.
  */
void BRepModel::addToGroup(const smtk::util::UUID& groupId, const UUIDs& uids)
{
  UUIDWithEntity result = this->m_topology->find(groupId);
  if (result == this->m_topology->end())
    {
    return;
    }

  for (UUIDs::const_iterator it = uids.begin(); it != uids.end(); ++it)
    {
    result->second.appendRelation(*it);
    }
  this->insertEntityReferences(result);
}

/** @name Model property accessors.
  *
  */
///@{
void BRepModel::setFloatProperty(
  const smtk::util::UUID& entity,
  const std::string& propName,
  smtk::model::Float propValue)
{
  smtk::model::FloatList tmp;
  tmp.push_back(propValue);
  this->setFloatProperty(entity, propName, tmp);
}

void BRepModel::setFloatProperty(
  const smtk::util::UUID& entity,
  const std::string& propName,
  const smtk::model::FloatList& propValue)
{
  if (!entity.isNull())
    {
    (*this->m_floatData)[entity][propName] = propValue;
    }
}

smtk::model::FloatList const& BRepModel::floatProperty(
  const smtk::util::UUID& entity, const std::string& propName) const
{
  if (!entity.isNull())
    {
    FloatData& floats((*this->m_floatData)[entity]);
    return floats[propName];
    }
  static FloatList dummy;
  return dummy;
}

smtk::model::FloatList& BRepModel::floatProperty(
  const smtk::util::UUID& entity, const std::string& propName)
{
  if (!entity.isNull())
    {
    FloatData& floats((*this->m_floatData)[entity]);
    return floats[propName];
    }
  static FloatList dummy;
  return dummy;
}

bool BRepModel::hasFloatProperty(
  const smtk::util::UUID& entity, const std::string& propName) const
{
  UUIDsToFloatData::const_iterator uit = this->m_floatData->find(entity);
  if (uit == this->m_floatData->end())
    {
    return false;
    }
  FloatData::const_iterator sit = uit->second.find(propName);
  // FIXME: Should we return true even when the array (*sit) is empty?
  return sit == uit->second.end() ? false : true;
}

bool BRepModel::removeFloatProperty(
  const smtk::util::UUID& entity,
  const std::string& propName)
{
  UUIDsToFloatData::iterator uit = this->m_floatData->find(entity);
  if (uit == this->m_floatData->end())
    {
    return false;
    }
  FloatData::iterator sit = uit->second.find(propName);
  if (sit == uit->second.end())
    {
    return false;
    }
  uit->second.erase(sit);
  return true;
}

const UUIDWithFloatProperties BRepModel::floatPropertiesForEntity(const smtk::util::UUID& entity) const
{
  return this->m_floatData->find(entity);
}

UUIDWithFloatProperties BRepModel::floatPropertiesForEntity(const smtk::util::UUID& entity)
{
  return this->m_floatData->find(entity);
}

void BRepModel::setStringProperty(
  const smtk::util::UUID& entity,
  const std::string& propName,
  const smtk::model::String& propValue)
{
  smtk::model::StringList tmp;
  tmp.push_back(propValue);
  this->setStringProperty(entity, propName, tmp);
}

void BRepModel::setStringProperty(
  const smtk::util::UUID& entity,
  const std::string& propName,
  const smtk::model::StringList& propValue)
{
  if (!entity.isNull())
    {
    (*this->m_stringData)[entity][propName] = propValue;
    }
}

smtk::model::StringList const& BRepModel::stringProperty(
  const smtk::util::UUID& entity, const std::string& propName) const
{
  if (!entity.isNull())
    {
    StringData& strings((*this->m_stringData)[entity]);
    return strings[propName];
    }
  static StringList dummy;
  return dummy;
}

smtk::model::StringList& BRepModel::stringProperty(
  const smtk::util::UUID& entity, const std::string& propName)
{
  if (!entity.isNull())
    {
    StringData& strings((*this->m_stringData)[entity]);
    return strings[propName];
    }
  static StringList dummy;
  return dummy;
}

bool BRepModel::hasStringProperty(
  const smtk::util::UUID& entity, const std::string& propName) const
{
  UUIDsToStringData::const_iterator uit = this->m_stringData->find(entity);
  if (uit == this->m_stringData->end())
    {
    return false;
    }
  StringData::const_iterator sit = uit->second.find(propName);
  // FIXME: Should we return true even when the array (*sit) is empty?
  return sit == uit->second.end() ? false : true;
}

bool BRepModel::removeStringProperty(
  const smtk::util::UUID& entity,
  const std::string& propName)
{
  UUIDsToStringData::iterator uit = this->m_stringData->find(entity);
  if (uit == this->m_stringData->end())
    {
    return false;
    }
  StringData::iterator sit = uit->second.find(propName);
  if (sit == uit->second.end())
    {
    return false;
    }
  uit->second.erase(sit);
  return true;
}

const UUIDWithStringProperties BRepModel::stringPropertiesForEntity(const smtk::util::UUID& entity) const
{
  return this->m_stringData->find(entity);
}

UUIDWithStringProperties BRepModel::stringPropertiesForEntity(const smtk::util::UUID& entity)
{
  return this->m_stringData->find(entity);
}

void BRepModel::setIntegerProperty(
  const smtk::util::UUID& entity,
  const std::string& propName,
  smtk::model::Integer propValue)
{
  smtk::model::IntegerList tmp;
  tmp.push_back(propValue);
  this->setIntegerProperty(entity, propName, tmp);
}

void BRepModel::setIntegerProperty(
  const smtk::util::UUID& entity,
  const std::string& propName,
  const smtk::model::IntegerList& propValue)
{
  if (!entity.isNull())
    {
    (*this->m_integerData)[entity][propName] = propValue;
    }
}

smtk::model::IntegerList const& BRepModel::integerProperty(
  const smtk::util::UUID& entity, const std::string& propName) const
{
  if (!entity.isNull())
    {
    IntegerData& integers((*this->m_integerData)[entity]);
    return integers[propName];
    }
  static IntegerList dummy;
  return dummy;
}

smtk::model::IntegerList& BRepModel::integerProperty(
  const smtk::util::UUID& entity, const std::string& propName)
{
  if (!entity.isNull())
    {
    IntegerData& integers((*this->m_integerData)[entity]);
    return integers[propName];
    }
  static IntegerList dummy;
  return dummy;
}

bool BRepModel::hasIntegerProperty(
  const smtk::util::UUID& entity, const std::string& propName) const
{
  UUIDsToIntegerData::const_iterator uit = this->m_integerData->find(entity);
  if (uit == this->m_integerData->end())
    {
    return false;
    }
  IntegerData::const_iterator sit = uit->second.find(propName);
  // FIXME: Should we return true even when the array (*sit) is empty?
  return sit == uit->second.end() ? false : true;
}

bool BRepModel::removeIntegerProperty(
  const smtk::util::UUID& entity,
  const std::string& propName)
{
  UUIDsToIntegerData::iterator uit = this->m_integerData->find(entity);
  if (uit == this->m_integerData->end())
    {
    return false;
    }
  IntegerData::iterator sit = uit->second.find(propName);
  if (sit == uit->second.end())
    {
    return false;
    }
  uit->second.erase(sit);
  return true;
}

const UUIDWithIntegerProperties BRepModel::integerPropertiesForEntity(const smtk::util::UUID& entity) const
{
  return this->m_integerData->find(entity);
}

UUIDWithIntegerProperties BRepModel::integerPropertiesForEntity(const smtk::util::UUID& entity)
{
  return this->m_integerData->find(entity);
}
///@}

/// Attempt to find a model owning the given entity.
smtk::util::UUID BRepModel::modelOwningEntity(const smtk::util::UUID& ent)
{
  smtk::util::UUID uid(ent);
  UUIDWithEntity it = this->m_topology->find(uid);
  if (it != this->m_topology->end())
    {
    // If we have a use or a shell, get the associated cell, if any
    smtk::model::BitFlags etype = it->second.entityFlags();
    switch (etype & ENTITY_MASK)
      {
    case GROUP_ENTITY:
      // Assume the first relationship that is a group or model is our owner.
      // Keep going up parent groups until we hit the top.
      for (
        smtk::model::UUIDArray::iterator sit = it->second.relations().begin();
        sit != it->second.relations().end();
        ++sit)
        {
        UUIDWithEntity subentity = this->topology().find(*sit);
        if (subentity != this->topology().end() && subentity->first != uid)
          {
          if (subentity->second.entityFlags() & MODEL_ENTITY)
            return subentity->first;
          if (subentity->second.entityFlags() & GROUP_ENTITY)
            { // Switch to finding relations of the group (assume it is our parent)
            uid = subentity->first;
            it = this->m_topology->find(uid);
            sit = it->second.relations().begin();
            }
          }
        }
      break;
    case INSTANCE_ENTITY:
      // Look for any relationship. We assume the first one is our prototype.
      for (
        smtk::model::UUIDArray::iterator sit = it->second.relations().begin();
        sit != it->second.relations().end();
        ++sit)
        {
        UUIDWithEntity subentity = this->topology().find(*sit);
        if (subentity != this->topology().end() && subentity->first != uid)
          {
          if (subentity->second.entityFlags() & MODEL_ENTITY)
            return subentity->first;
          return this->modelOwningEntity(subentity->first);
          break;
          }
        }
      break;
    case SHELL_ENTITY:
      // Loop for a relationship to a use.
      for (
        smtk::model::UUIDArray::iterator sit = it->second.relations().begin();
        sit != it->second.relations().end();
        ++sit)
        {
        UUIDWithEntity subentity = this->topology().find(*sit);
        if (
          subentity != this->topology().end() &&
          smtk::model::isUseEntity(subentity->second.entityFlags()))
          {
          it = subentity;
          break;
          }
        }
      // Now fall through and look for the use's relationship to a cell.
    case USE_ENTITY:
      // Look for a relationship to a cell
      for (
        smtk::model::UUIDArray::iterator sit = it->second.relations().begin();
        sit != it->second.relations().end();
        ++sit)
        {
        UUIDWithEntity subentity = this->topology().find(*sit);
        if (
          subentity != this->topology().end() &&
          smtk::model::isCellEntity(subentity->second.entityFlags()))
          {
          it = subentity;
          break;
          }
        }
      break;
    case MODEL_ENTITY:
      // For models, life is tricky. Without arrangement information, we cannot
      // know whether a related model is a child or a parent. Two models might
      // point to each other, which could throw us into an infinite loop. So,
      // we attempt to cast ourselves to Storage and identify a parent model.
        {
        StoragePtr store = smtk::dynamic_pointer_cast<Storage>(shared_from_this());
        if (store)
          {
          ModelEntities parents;
          CursorArrangementOps::appendAllRelations(ModelEntity(store,ent), EMBEDDED_IN, parents);
          if (!parents.empty())
            return parents[0].entity();
          }
        }
      break;
    // Remaining types should all have a direct relationship with a model if they are free:
    default:
    case CELL_ENTITY:
      break;
      }
    // Now look for a direct relationship with a model.
    // If none exists, look for relationships with higher-dimensional entities
    // and check *them* for models.
    int dim;
    smtk::util::UUIDs uids;
    uids.insert(it->first);
    for (dim = it->second.dimension(); dim >= 0 && dim < 4; ++dim)
      {
      for (smtk::util::UUIDs::iterator uit = uids.begin(); uit != uids.end(); ++uit)
        {
        Entity* bordEnt = this->findEntity(*uit);
        if (!bordEnt) continue;
        for (smtk::util::UUIDArray::iterator rit = bordEnt->relations().begin(); rit != bordEnt->relations().end(); ++rit)
          {
          Entity* relEnt = this->findEntity(*rit);
          if (relEnt && (relEnt->entityFlags() & MODEL_ENTITY))
            {
            return *rit;
            }
          }
        }
      // FIXME: This is slow. Avoid calling bordantEntities().
      uids = this->bordantEntities(uids, dim + 1);
      }
    }
  return smtk::util::UUID::null();
}

/**\brief Return a bridge associated with the given model.
  *
  * Because modeling operations require access to the un-transcribed model
  * and the original modeling kernel, operations are associated with the
  * bridge that performs the transcription.
  *
  * \sa BridgeBase
  */
BridgeBasePtr BRepModel::bridgeForModel(const smtk::util::UUID& uid)
{
  // See if the passed entity has a bridge.
  UUIDsToBridges::iterator it = this->m_modelBridges.find(uid);
  if (it != this->m_modelBridges.end())
    return it->second;

  // Nope? OK, see if we can go up a tree of models to find a
  // parent that does have a bridge.
  smtk::util::UUID entry(uid);
  while (
    (entry = this->modelOwningEntity(entry)) &&
    ((it = this->m_modelBridges.find(entry)) == this->m_modelBridges.end()))
    /* keep trying */
    ;
  if (it != this->m_modelBridges.end())
    return it->second;

  // Nope? Return the default bridge.
  return this->m_defaultBridge;
}

/**\brief Associate a bridge with the given model.
  *
  * The \a uid and all its children (excepting those which have their
  * own bridge set) will be associated with the given \a bridge.
  * If \a uid already had a bridge entry, it will be changed to the
  * specified \a bridge.
  *
  * \sa BridgeBase
  */
void BRepModel::setBridgeForModel(
  BridgeBasePtr bridge, const smtk::util::UUID& uid)
{
  this->m_modelBridges[uid] = bridge;
}

/**\brief Assign a string property named "name" to each entity without one.
  *
  * If a model can be identified as owning the entity, the default name
  * assigned to the entity will be the model's name followed by a command
  * and then the name for the entity.
  * If entities without names have an owning model, then per-model counters
  * are used to number entities of the same type (e.g., "Face 13", "Edge 42").
  * Otherwise, the trailing digits of entity UUIDs are used.
  */
void BRepModel::assignDefaultNames()
{
  UUIDWithEntity it;
  for (it = this->m_topology->begin(); it != this->m_topology->end(); ++it)
    {
    if (!this->hasStringProperty(it->first, "name"))
      {
      this->assignDefaultName(it->first, it->second.entityFlags());
      }
    }
}

std::string BRepModel::assignDefaultName(const smtk::util::UUID& uid)
{
  UUIDWithEntity it = this->m_topology->find(uid);
  if (it != this->m_topology->end())
    {
    return this->assignDefaultName(it->first, it->second.entityFlags());
    }
  return std::string();
}

std::string BRepModel::assignDefaultName(const smtk::util::UUID& uid, BitFlags entityFlags)
{
  // If this entity is a model, give it a top-level name
  // (even if it is a submodel of some other model -- for brevity).
  if (entityFlags & MODEL_ENTITY)
    {
    std::string tmpName;
    if (!this->hasStringProperty(uid,"name"))
      {
      std::ostringstream defaultName;
      defaultName << "Model ";
      int count = this->m_modelCount++;
      char hexavigesimal[8]; // 7 hexavigesimal digits will cover us up to 2**31.
      int i;
      for (i = 0; count > 0 && i < 7; ++i)
        {
        --count;
        hexavigesimal[i] = 'A' + count % 26;
        count /= 26;
        }
      for (--i; i >= 0; --i)
        {
        defaultName << hexavigesimal[i];
        }
      tmpName = defaultName.str();
      this->setStringProperty(uid, "name", tmpName);
      }
    else
      {
      tmpName = this->stringProperty(uid, "name")[0];
      }
    return tmpName;
    }
  // Otherwise, use the "owning" model as part of the default name
  // for the entity. First, get the name of the entity's owner:
  smtk::util::UUID owner(
    this->modelOwningEntity(uid));
  std::string ownerName;
  if (owner)
    {
    if (this->hasStringProperty(owner, "name"))
      {
      ownerName = this->stringProperty(owner, "name")[0];
      }
    else
      {
      ownerName = this->assignDefaultName(
        owner, this->findEntity(owner)->entityFlags());
      }
    ownerName += ", ";
    }
  // Now get the owner's list of per-type counters:
  IntegerList& counts(
    this->entityCounts(
      owner, entityFlags));
  // Compose a name from the owner and counters:
  std::string defaultName =
    counts.empty() ?
    this->shortUUIDName(uid, entityFlags) :
    ownerName + Entity::defaultNameFromCounters(entityFlags, counts);
  this->setStringProperty(uid, "name", defaultName);
  return defaultName;
}

std::string BRepModel::shortUUIDName(const smtk::util::UUID& uid, BitFlags entityFlags)
{
  std::string name = Entity::flagSummaryHelper(entityFlags);
  name += "..";
  std::string uidStr = uid.toString();
  name += uidStr.substr(uidStr.size() - 4);
  return name;
}

IntegerList& BRepModel::entityCounts(
  const smtk::util::UUID& modelId, BitFlags entityFlags)
{
  switch (entityFlags & ENTITY_MASK)
    {
  case CELL_ENTITY:
    return this->integerProperty(modelId, "cell_counters");
  case USE_ENTITY:
    return this->integerProperty(modelId, "use_counters");
  case SHELL_ENTITY:
    return this->integerProperty(modelId, "shell_counters");
  case GROUP_ENTITY:
    return this->integerProperty(modelId, "group_counters");
  case MODEL_ENTITY:
    return this->integerProperty(modelId, "model_counters");
  case INSTANCE_ENTITY:
    return this->integerProperty(modelId, "instance_counters");
  default:
    break;
    }
  return this->integerProperty(modelId, "invalid_counters");
}

/**\brief Initialize storage outside of the topology() table for a new entity.
  *
  * This is an internal method invoked by setEntity and SetEntityOfTypeAndDimension.
  */
void BRepModel::prepareForEntity(std::pair<smtk::util::UUID,Entity>& entry)
{
  if ((entry.second.entityFlags() & CELL_2D) == CELL_2D)
    {
    Storage* store = dynamic_cast<Storage*>(this);
    if (store && !store->hasArrangementsOfKindForEntity(entry.first, HAS_USE))
      {
      // Create arrangements to hold face-uses:
      store->arrangeEntity(entry.first, HAS_USE, Arrangement::CellHasUseWithIndexSenseAndOrientation(-1, 0, NEGATIVE));
      store->arrangeEntity(entry.first, HAS_USE, Arrangement::CellHasUseWithIndexSenseAndOrientation(-1, 1, POSITIVE));
      }
    }
  else if (entry.second.entityFlags() & USE_ENTITY)
    {
    Storage* store = dynamic_cast<Storage*>(this);
    if (store && !store->hasArrangementsOfKindForEntity(entry.first, HAS_SHELL))
      {
      // Create arrangement to hold parent shell:
      store->arrangeEntity(entry.first, HAS_SHELL, Arrangement::UseHasShellWithIndex(-1));
      }
    }
  else if ((entry.second.entityFlags() & MODEL_ENTITY) == MODEL_ENTITY)
    {
    // New models keep counters indicating their local entity counters
    Integer topoCountsData[] = {0, 0, 0, 0, 0, 0};
    Integer groupCountsData[] = {0, 0, 0};
    Integer otherCountsData[] = {0};
    IntegerList topoCounts(
      topoCountsData,
      topoCountsData + sizeof(topoCountsData)/sizeof(topoCountsData[0]));
    IntegerList groupCounts(
      groupCountsData,
      groupCountsData + sizeof(groupCountsData)/sizeof(groupCountsData[0]));
    IntegerList otherCounts(
      otherCountsData,
      otherCountsData + sizeof(otherCountsData)/sizeof(otherCountsData[0]));
    this->setIntegerProperty(entry.first, "cell_counters", topoCounts);
    this->setIntegerProperty(entry.first, "use_counters", topoCounts);
    this->setIntegerProperty(entry.first, "shell_counters", topoCounts);
    this->setIntegerProperty(entry.first, "group_counters", groupCounts);
    this->setIntegerProperty(entry.first, "model_counters", otherCounts);
    this->setIntegerProperty(entry.first, "instance_counters", otherCounts);
    this->setIntegerProperty(entry.first, "invalid_counters", otherCounts);
    }
}

  } // model namespace
} // smtk namespace
