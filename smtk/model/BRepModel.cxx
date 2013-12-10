#include "smtk/model/BRepModel.h"

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
/// Insert a new cell of the specified \a dimension, returning an iterator with a new, unique UUID.
BRepModel::iter_type BRepModel::insertEntityOfTypeAndDimension(BitFlags entityFlags, int dim)
{
  UUID actual;
  do
    {
    actual = this->m_uuidGenerator.random();
    }
  while (this->m_topology->find(actual) != this->m_topology->end());
  return this->setEntityOfTypeAndDimension(actual, entityFlags, dim);
}

/// Insert the specified cell, returning an iterator with a new, unique UUID.
BRepModel::iter_type BRepModel::insertEntity(Entity& c)
{
  UUID actual;
  do
    {
    actual = this->m_uuidGenerator.random();
    }
  while (this->m_topology->find(actual) != this->m_topology->end());
  return this->setEntity(actual, c);
}

/**\brief Map a new cell of the given \a dimension to the \a uid.
  *
  * Passing a non-unique \a uid is an error here and will throw an exception.
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
  return this->m_topology->insert(entry).first;
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
int BRepModel::type(const smtk::util::UUID& ofEntity) const
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
UUIDs BRepModel::bordantEntities(const UUID& ofEntity, int ofDimension)
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
UUIDs BRepModel::bordantEntities(const UUIDs& ofEntities, int ofDimension)
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
UUIDs BRepModel::boundaryEntities(const UUID& ofEntity, int ofDimension)
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
UUIDs BRepModel::boundaryEntities(const UUIDs& ofEntities, int ofDimension)
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

/// Remove an entity \a uid from storage and ensure that all of its references contain <b>no</b> reference to it.
bool BRepModel::removeEntity(const smtk::util::UUID& uid)
{
  UUIDWithEntity it = this->m_topology->find(uid);
  if (it != this->m_topology->end())
    {
    this->removeEntityReferences(it);
    return true;
    }
  return false;
}

/**\brief Add entities (specified by their \a uids) to the given group (\a groupId).
  *
  * This will append \a groupId to each entity in \a uids.
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

/// Add a vertex to storage (without any relationships)
smtk::util::UUID BRepModel::addVertex()
{
  return this->addEntityOfTypeAndDimension(CELL_ENTITY, 0);
}

/// Add a vertex to storage (without any relationships)
smtk::util::UUID BRepModel::addEdge()
{
  return this->addEntityOfTypeAndDimension(CELL_ENTITY, 1);
}

/// Add a vertex to storage (without any relationships)
smtk::util::UUID BRepModel::addFace()
{
  return this->addEntityOfTypeAndDimension(CELL_ENTITY, 2);
}

/// Add a vertex to storage (without any relationships)
smtk::util::UUID BRepModel::addVolume()
{
  return this->addEntityOfTypeAndDimension(CELL_ENTITY, 3);
}

/// Add a vertex use to storage (without any relationships)
smtk::util::UUID BRepModel::addVertexUse()
{
  return this->addEntityOfTypeAndDimension(USE_ENTITY, 0);
}

/// Add a vertex use to storage (without any relationships)
smtk::util::UUID BRepModel::addEdgeUse()
{
  return this->addEntityOfTypeAndDimension(USE_ENTITY, 1);
}

/// Add a vertex use to storage (without any relationships)
smtk::util::UUID BRepModel::addFaceUse()
{
  return this->addEntityOfTypeAndDimension(USE_ENTITY, 2);
}

/// Add a 0/1-d shell (a vertex chain) to storage (without any relationships)
smtk::util::UUID BRepModel::addChain()
{
  return this->addEntityOfTypeAndDimension(SHELL_ENTITY | DIMENSION_0 | DIMENSION_1, -1);
}

/// Add a 0/1-d shell (a vertex chain) to storage (without any relationships)
smtk::util::UUID BRepModel::addLoop()
{
  return this->addEntityOfTypeAndDimension(SHELL_ENTITY | DIMENSION_1 | DIMENSION_2, -1);
}

/// Add a 0/1-d shell (a vertex chain) to storage (without any relationships)
smtk::util::UUID BRepModel::addShell()
{
  return this->addEntityOfTypeAndDimension(SHELL_ENTITY | DIMENSION_2 | DIMENSION_3, -1);
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
smtk::util::UUID BRepModel::addGroup(int extraFlags, const std::string& name)
{
  smtk::util::UUID uid = this->addEntityOfTypeAndDimension(GROUP_ENTITY | extraFlags, -1);
  this->setStringProperty(uid, "name", name);
  return uid;
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
smtk::util::UUID BRepModel::addModel(
  int parametricDim, int embeddingDim, const std::string& name)
{
  smtk::util::UUID uid = this->addEntityOfTypeAndDimension(MODEL_ENTITY, parametricDim);
  if (embeddingDim > 0)
    {
    this->setIntegerProperty(uid, "embedding dimension", embeddingDim);
    }
  std::string tmpName(name);
  if (tmpName.empty())
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
    }
  this->setStringProperty(uid, "name", tmpName);
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
  this->setIntegerProperty(uid, "cell_counters", topoCounts);
  this->setIntegerProperty(uid, "use_counters", topoCounts);
  this->setIntegerProperty(uid, "shell_counters", topoCounts);
  this->setIntegerProperty(uid, "group_counters", groupCounts);
  this->setIntegerProperty(uid, "model_counters", otherCounts);
  this->setIntegerProperty(uid, "instance_counters", otherCounts);
  this->setIntegerProperty(uid, "invalid_counters", otherCounts);
  return uid;
}

/// Attempt to find a model owning the given entity.
smtk::util::UUID BRepModel::modelOwningEntity(const smtk::util::UUID& uid)
{
  UUIDWithEntity it = this->m_topology->find(uid);
  if (it != this->m_topology->end())
    {
    int dim;
    smtk::util::UUIDs uids;
    uids.insert(uid);
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
  IntegerList& counts(
    this->entityCounts(
      this->modelOwningEntity(uid),
      entityFlags));
  std::string defaultName =
    counts.empty() ?
    this->shortUUIDName(uid, entityFlags) :
    Entity::defaultNameFromCounters(entityFlags, counts);
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

  } // model namespace
} // smtk namespace
