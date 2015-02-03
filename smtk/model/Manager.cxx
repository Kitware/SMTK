//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Manager.h"

#include "smtk/attribute/System.h"
#include "smtk/attribute/Attribute.h"

#include "smtk/model/AttributeAssignments.h"
#include "smtk/model/SessionRef.h"
#include "smtk/model/Chain.h"
#include "smtk/model/DefaultSession.h"
#include "smtk/model/EntityRefArrangementOps.h"
#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Model.h"
#include "smtk/model/Shell.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/VertexUse.h"
#include "smtk/model/Volume.h"
#include "smtk/model/VolumeUse.h"

#include <algorithm>
#include <set>
#include <map>
#include <vector>

#include <sstream>

//#include <boost/variant.hpp>

using namespace std;
using namespace smtk::common;

namespace smtk {
  namespace model {

/**@name Constructors and destructors.
  *\brief Model manager instances should always be created using the static create() method.
  *
  */
//@{
/// Create a default, empty model manager.
Manager::Manager() :
  m_topology(new UUIDsToEntities),
  m_floatData(new UUIDsToFloatData),
  m_stringData(new UUIDsToStringData),
  m_integerData(new UUIDsToIntegerData),
  m_arrangements(new UUIDsToArrangements),
  m_tessellations(new UUIDsToTessellations),
  m_attributeAssignments(new UUIDsToAttributeAssignments),
  m_sessions(new UUIDsToSessions),
  m_attributeSystem(NULL),
  m_globalCounters(2,1) // first entry is session counter, second is model counter
{
  // TODO: throw() when topology == NULL?
}

/// Create a model manager using the given storage instances.
Manager::Manager(
  shared_ptr<UUIDsToEntities> inTopology,
  shared_ptr<UUIDsToArrangements> inArrangements,
  shared_ptr<UUIDsToTessellations> tess,
  shared_ptr<UUIDsToAttributeAssignments> attribs)
  :
    m_topology(inTopology),
    m_floatData(new UUIDsToFloatData),
    m_stringData(new UUIDsToStringData),
    m_integerData(new UUIDsToIntegerData),
    m_arrangements(inArrangements),
    m_tessellations(tess),
    m_attributeAssignments(attribs),
    m_sessions(new UUIDsToSessions),
    m_attributeSystem(NULL),
    m_globalCounters(2,1) // first entry is session counter, second is model counter
{
}

/// Destroying a model manager requires us to release the default attribute manager..
Manager::~Manager()
{
  if (this->m_defaultSession)
    {
    // NB: We must pass "false" for the expungeSession argument because
    // the manager may have 0 shared-pointer references at this point
    // and thus cannot construct trigger callback cursors to notify
    // listeners that deletions are occurring.
    this->unregisterSession(this->m_defaultSession, false);
    }
  this->setAttributeSystem(NULL, false);
}
//@}

/**@name Direct member access.
  *\brief These methods provide direct access to the class's storage.
  *
  */
//@{
UUIDsToEntities& Manager::topology()
{
  return *this->m_topology.get();
}

const UUIDsToEntities& Manager::topology() const
{
  return *this->m_topology.get();
}

UUIDsToArrangements& Manager::arrangements()
{
  return *this->m_arrangements.get();
}

const UUIDsToArrangements& Manager::arrangements() const
{
  return *this->m_arrangements.get();
}

UUIDsToTessellations& Manager::tessellations()
{
  return *this->m_tessellations.get();
}

const UUIDsToTessellations& Manager::tessellations() const
{
  return *this->m_tessellations.get();
}

UUIDsToAttributeAssignments& Manager::attributeAssignments()
{
  return *this->m_attributeAssignments;
}

const UUIDsToAttributeAssignments& Manager::attributeAssignments() const
{
  return *this->m_attributeAssignments;
}
//@}

/**\brief Remove the entity with the given \a uid.
  *
  * Returns true upon success, false when the entity did not exist.
  *
  * Note that the implementation is aware of when the Manager
  * is actually a Manager and removes storage from the Manager as
  * well (including tessellation data).
  *
  * **Warning**: Invoking this method naively will likely result
  * in an inconsistent solid model. This does not cascade
  * any changes required to remove dependent entities (i.e.,
  * removing a face does not remove any face-uses or shells that
  * the face participated in, potentially leaving an improper volume
  * boundary). The application is expected to perform further
  * operations to keep the model valid.
  */
bool Manager::erase(const UUID& uid)
{
  UUIDWithEntity ent = this->m_topology->find(uid);
  if (ent == this->m_topology->end())
    return false;

  bool isModelEnt = isModel(ent->second.entityFlags());

  // Trigger an event before the erasure so the observers
  // have a chance to see what's about to disappear.
  this->trigger(std::make_pair(DEL_EVENT, ENTITY_ENTRY),
    EntityRef(this->shared_from_this(), uid));

  UUIDWithArrangementDictionary ad = this->arrangements().find(uid);
  if (ad != this->arrangements().end())
    {
    ArrangementKindWithArrangements ak;
    do
      {
      ak = ad->second.begin();
      if (ak == ad->second.end())
        break;
      Arrangements::size_type aidx = ak->second.size();
      for (; aidx > 0; --aidx)
        this->unarrangeEntity(uid, ak->first, static_cast<int>(aidx - 1), false);
      ad = this->arrangements().find(uid); // iterator may be invalidated by unarrangeEntity.
      }
    while (ad != this->arrangements().end());
    }
  this->tessellations().erase(uid);
  this->attributeAssignments().erase(uid);

  // TODO: If this entity is a model and has an entry in m_modelSessions,
  //       we should verify that any submodels retain a reference to the
  //       Session in m_modelSessions.

  // Before removing the entity, loop through its relations and
  // make sure none of them retain any references back to \a uid.
  // However, we cannot erase entries in relatedEntity->relations()
  // because relatedEntity's arrangements reference them by integer
  // index. Thus, we call elideEntityReferences rather than removeEntityReferences.
  this->elideEntityReferences(ent);

  // TODO: Notify observers of property removal?
  this->m_floatData->erase(uid);
  this->m_stringData->erase(uid);
  this->m_integerData->erase(uid);

  // TODO: Notify model of entity removal?
  this->m_topology->erase(uid);

  // If the entity was a model, remove any session entry for it.
  if (isModelEnt)
    this->m_modelSessions.erase(uid);

  return true;
}

/**\brief A convenience method for erasing an entity from storage.
  *
  */
bool Manager::erase(const EntityRef& entityref)
{
  return this->Manager::erase(entityref.entity());
}

/**\brief A convenience method for erasing a model and its children.
  *
  * This removes the model plus all of its free cells, groups, and
  * submodels from storage.
  * This method will have no effect given an invalid model entity.
  */
bool Manager::eraseModel(const Model& model)
{
  if (!model.isValid())
    return false;

  CellEntities free = model.cells();
  for (CellEntities::iterator fit = free.begin(); fit != free.end(); ++fit)
    {
    EntityRefs bdys = fit->lowerDimensionalBoundaries(-1);
    for (EntityRefs::iterator bit = bdys.begin(); bit != bdys.end(); ++bit)
      {
      //std::cout << "Erasing " << bit->flagSummary(0) << " " << bit->entity() << "\n";
      this->erase(bit->entity());
      }
    //std::cout << "Erasing " << fit->flagSummary(0) << " " << fit->entity() << "\n";
    this->erase(fit->entity());
    }

  GroupEntities grps = model.groups();
  for (GroupEntities::iterator git = grps.begin(); git != grps.end(); ++git)
    {
    EntityRefs members = git->members<EntityRefs>();
    for (EntityRefs::iterator mit = members.begin(); mit != members.end(); ++mit)
      {
      //std::cout << "Erasing " << mit->flagSummary(0) << " " << mit->entity() << "\n";
      this->erase(mit->entity());
      }
    //std::cout << "Erasing " << git->flagSummary(0) << " " << git->entity() << "\n";
    this->erase(git->entity());
    }

  //std::cout << "Erasing " << model.flagSummary(0) << " " << model.entity() << "\n";
  this->erase(model.entity());

  return true;
}

/**\brief Set the attribute manager.
  *
  * This is an error if the manager already has a non-null
  * reference to a different model manager instance.
  *
  * If this manager is associated with a different attribute system,
  * that attribute system is detached (its model manager reference
  * set to null) and all attribute associations in the model manager
  * are erased.
  * This is not an error, but a warning message will be generated.
  *
  * On error, false is returned, an error message is generated,
  * and no change is made to the attribute system.
  */
bool Manager::setAttributeSystem(smtk::attribute::System* attSys, bool reverse)
{
  if (attSys)
    {
    smtk::model::Manager* attSysModelMgr = attSys->refModelManager().get();
    if (attSysModelMgr && attSysModelMgr != this)
      {
      return false;
      }
    }
  if (this->m_attributeSystem && this->m_attributeSystem != attSys)
    {
    // Only warn when (a) the new manager is non-NULL and (b) we
    // have at least 1 attribute association.
    if (!this->m_attributeAssignments->empty() && attSys)
      {
      smtkInfoMacro(this->m_log,
        "WARNING: Changing attribute managers.\n"
        "         Current attribute associations cleared.\n");
      this->m_attributeAssignments->clear();
      }
    if (reverse)
      this->m_attributeSystem->setRefModelManager(ManagerPtr());
    }
  this->m_attributeSystem = attSys;
  if (this->m_attributeSystem && reverse)
    this->m_attributeSystem->setRefModelManager(shared_from_this());
  return true;
}

/**\brief Return the attribute manager associated with this model manager.
  *
  */
smtk::attribute::System* Manager::attributeSystem() const
{
  return this->m_attributeSystem;
}

/// Entity construction
//@{
/// Return a currently-unused UUID (guaranteed not to collide if inserted immediately).
UUID Manager::unusedUUID()
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
Manager::iter_type Manager::insertEntityOfTypeAndDimension(BitFlags entityFlags, int dim)
{
  UUID actual = this->unusedUUID();
  return this->setEntityOfTypeAndDimension(actual, entityFlags, dim);
}

/// Insert the specified cell, returning an iterator with a new, unique UUID.
Manager::iter_type Manager::insertEntity(Entity& c)
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
  * If the Manager may be cast to a Manager instance and an entity
  * is expected to have a known, fixed number of arrangements of some sort,
  * those are created here so that entityrefs may always rely on their existence
  * even in the absence of the related UUIDs appearing in the entity's relations.
  * For face cells (CELL_2D) entites, two HAS_USE Arrangements are created to
  * reference FaceUse instances.
  */
Manager::iter_type Manager::setEntityOfTypeAndDimension(const UUID& uid, BitFlags entityFlags, int dim)
{
  UUIDsToEntities::iterator it;
  if (uid.isNull())
    {
    std::ostringstream msg;
    msg << "Nil UUID";
    throw msg.str();
    }
  if ((it = this->m_topology->find(uid)) != this->m_topology->end() && it->second.dimension() != dim)
    {
    std::ostringstream msg;
    msg << "Duplicate UUID '" << uid << "' of different dimension " << it->second.dimension() << " != " << dim;
    throw msg.str();
    }
  std::pair<UUID,Entity> entry(uid,Entity(entityFlags, dim));
  this->prepareForEntity(entry);
  std::pair<Manager::iter_type,bool> result = this->m_topology->insert(entry);

  if (result.second)
    {
    this->trigger(std::make_pair(ADD_EVENT, ENTITY_ENTRY),
      EntityRef(this->shared_from_this(), uid));
    }

  return result.first;
}

/**\brief Map the specified cell \a c to the given \a uid.
  *
  * Passing a nil or non-unique \a uid is an error here and will throw an exception.
  */
Manager::iter_type Manager::setEntity(const UUID& uid, Entity& c)
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
UUID Manager::addEntityOfTypeAndDimension(BitFlags entityFlags, int dim)
{
  return this->insertEntityOfTypeAndDimension(entityFlags, dim)->first;
}

/// A wrappable version of InsertEntity
UUID Manager::addEntity(Entity& cell)
{
  return this->insertEntity(cell)->first;
}

/// A wrappable version of SetEntityOfTypeAndDimension
UUID Manager::addEntityOfTypeAndDimensionWithUUID(const UUID& uid, BitFlags entityFlags, int dim)
{
  return this->setEntityOfTypeAndDimension(uid, entityFlags, dim)->first;
}

/// A wrappable version of SetEntity
UUID Manager::addEntityWithUUID(const UUID& uid, Entity& cell)
{
  return this->setEntity(uid, cell)->first;
}
//@}

/// Shortcuts for inserting cells with default entity flags.
//@{
Manager::iter_type Manager::insertCellOfDimension(int dim)
{
  return this->insertEntityOfTypeAndDimension(CELL_ENTITY, dim);
}

Manager::iter_type Manager::setCellOfDimension(const UUID& uid, int dim)
{
  return this->setEntityOfTypeAndDimension(uid, CELL_ENTITY, dim);
}

UUID Manager::addCellOfDimension(int dim)
{
  return this->addEntityOfTypeAndDimension(CELL_ENTITY, dim);
}

UUID Manager::addCellOfDimensionWithUUID(const UUID& uid, int dim)
{
  return this->addEntityOfTypeAndDimensionWithUUID(uid, CELL_ENTITY, dim);
}
//@}

/// Queries on entities belonging to the solid.
//@{
/// Return the type of entity that the link represents.
BitFlags Manager::type(const UUID& ofEntity) const
{
  UUIDsToEntities::iterator it = this->m_topology->find(ofEntity);
  return (it == this->m_topology->end() ? INVALID : it->second.entityFlags());
}

/// Return the dimension of the manifold that the passed entity represents.
int Manager::dimension(const UUID& ofEntity) const
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
std::string Manager::name(const UUID& ofEntity) const
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
  return Manager::shortUUIDName(it->first, it->second.entityFlags());
}

/**\brief Return the (Dimension+1 or higher)-entities that are the immediate bordants of the passed entity.
  *
  * \sa HigherDimensionalBoundaries
  */
UUIDs Manager::bordantEntities(const UUID& ofEntity, int ofDimension) const
{
  UUIDs result;
  UUIDsToEntities::const_iterator it = this->m_topology->find(ofEntity);
  if (it == this->m_topology->end())
    {
    return result;
    }
  if (ofDimension >= 0 && it->second.dimension() >= ofDimension)
    {
    // can't ask for "higher" dimensional boundaries that are lower than the dimension of this cell.
    return result;
    }
  UUIDsToEntities::const_iterator other;
  for (UUIDArray::const_iterator ai = it->second.relations().begin(); ai != it->second.relations().end(); ++ai)
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
UUIDs Manager::bordantEntities(const UUIDs& ofEntities, int ofDimension) const
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
UUIDs Manager::boundaryEntities(const UUID& ofEntity, int ofDimension) const
{
  UUIDs result;
  UUIDsToEntities::const_iterator it = this->m_topology->find(ofEntity);
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
  for (UUIDArray::const_iterator ai = it->second.relations().begin(); ai != it->second.relations().end(); ++ai)
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
UUIDs Manager::boundaryEntities(const UUIDs& ofEntities, int ofDimension) const
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
  * Unlike Manager::boundaryEntities(), this method will search the boundaries
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
UUIDs Manager::lowerDimensionalBoundaries(const UUID& ofEntity, int lowerDimension)
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
      UUIDs tmp = this->boundaryEntities(result, currentDim);
      if (lowerDimension >= 0)
        result.clear();
      result.insert(tmp.begin(), tmp.end());
      }
    }
  return result;
}

/**\brief Return higher-dimensional bordants of the passed d-dimensional entity.
  *
  * \a higherDimension may be any dimension > d.
  * Unlike Manager::bordantEntities(), this method will search the bordants
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
UUIDs Manager::higherDimensionalBordants(const UUID& ofEntity, int higherDimension)
{
  UUIDs result;
  UUIDsToEntities::iterator it = this->m_topology->find(ofEntity);
  if (it == this->m_topology->end())
    {
    return result;
    }
  if (higherDimension >= 0 && it->second.dimension() >= higherDimension)
    {
    // do nothing
    }
  else
    {
    int currentDim = it->second.dimension() + 1;
    int delta = higherDimension < 0 ? 4 : higherDimension - currentDim;
    result = this->bordantEntities(ofEntity, currentDim++);
    for (int i = delta; i > 0; --i, ++currentDim)
      {
      UUIDs tmp = this->bordantEntities(result, currentDim);
      if (higherDimension >= 0)
        result.clear();
      result.insert(tmp.begin(), tmp.end());
      }
    }
  return result;
}

/// Return entities of the requested dimension that share a boundary relationship with the passed entity.
UUIDs Manager::adjacentEntities(const UUID& ofEntity, int ofDimension)
{
  // FIXME: Implement adjacency
  (void)ofEntity;
  (void)ofDimension;
  UUIDs result;
  return result;
}

/// Return all entities of the requested dimension that are present in the solid.
UUIDs Manager::entitiesMatchingFlags(BitFlags mask, bool exactMatch)
{
  UUIDs result;
  for (UUIDWithEntity it = this->m_topology->begin(); it != this->m_topology->end(); ++it)
    {
    BitFlags masked = it->second.entityFlags() & mask;
    if ((masked && mask == ANY_ENTITY) ||
      (!exactMatch && masked) ||
      (exactMatch && masked == mask))
      {
      result.insert(it->first);
      }
    }
  return result;
}

/// Return all entities of the requested dimension that are present in the solid.
UUIDs Manager::entitiesOfDimension(int dim)
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

/**\brief Return the smtk::model::Entity associated with \a uid (or NULL).
  *
  * Note that even the const version of this method may invalidate other
  * pointers to Entity records since it may ask a Session instance to fetch
  * a dangling UUID (one marked as existing but un-transcribed) and insert
  * the Entity into Manager. If it is important that Entity pointers remain
  * valid, call with the second argument (\a trySessions) set to false.
  */
//@{
const Entity* Manager::findEntity(const UUID& uid, bool trySessions) const
{
  UUIDWithEntity it = this->m_topology->find(uid);
  if (it == this->m_topology->end())
    {
    // Not in storage... is it in any session's dangling entity list?
    // We use an evil const-cast here because we are working under the fiction
    // that fetching an entity that exists (even if it hasn't been transcribed
    // yet) does not affect storage.
    ManagerPtr self = const_cast<Manager*>(this)->shared_from_this();
    if (trySessions)
      {
      UUIDsToSessions::iterator bit;
      for (bit = self->m_modelSessions.begin(); bit != self->m_modelSessions.end(); ++bit)
        {
        if (bit->second->transcribe(EntityRef(self, uid), SESSION_ENTITY_ARRANGED, true))
          {
          it = this->m_topology->find(uid);
          if (it != this->m_topology->end())
            return &it->second;
          }
        }
      }
    return NULL;
    }
  return &it->second;
}

Entity* Manager::findEntity(const UUID& uid, bool trySessions)
{
  UUIDWithEntity it = this->m_topology->find(uid);
  if (it == this->m_topology->end())
    { // Not in storage... is it in any session's dangling entity list?
    ManagerPtr self = const_cast<Manager*>(this)->shared_from_this();
    if (trySessions && self)
      {
      UUIDsToSessions::iterator bit;
      for (bit = self->m_modelSessions.begin(); bit != self->m_modelSessions.end(); ++bit)
        {
        if (bit->second->transcribe(EntityRef(self, uid), SESSION_ENTITY_ARRANGED, true))
          {
          it = this->m_topology->find(uid);
          if (it != this->m_topology->end())
            return &it->second;
          }
        }
      }
    return NULL;
    }
  return &it->second;
}
//@}

/// Given an entity \a c, ensure that all of its references contain a reference to it.
void Manager::insertEntityReferences(const UUIDWithEntity& c)
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
void Manager::elideEntityReferences(const UUIDWithEntity& c)
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
          *rit = UUID::null();
          }
        }
      }
    }
}

/// Given an entity \a c, ensure that all of its references contain <b>no</b> reference to it.
void Manager::removeEntityReferences(const UUIDWithEntity& c)
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
  * that Manager::findOrAddEntityToGroup() does, since Manager
  * does not store Arrangement information.
  */
void Manager::addToGroup(const UUID& groupId, const UUIDs& uids)
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
void Manager::setFloatProperty(
  const UUID& entity,
  const std::string& propName,
  smtk::model::Float propValue)
{
  smtk::model::FloatList tmp;
  tmp.push_back(propValue);
  this->setFloatProperty(entity, propName, tmp);
}

void Manager::setFloatProperty(
  const UUID& entity,
  const std::string& propName,
  const smtk::model::FloatList& propValue)
{
  if (!entity.isNull())
    {
    (*this->m_floatData)[entity][propName] = propValue;
    }
}

smtk::model::FloatList const& Manager::floatProperty(
  const UUID& entity, const std::string& propName) const
{
  if (!entity.isNull())
    {
    FloatData& floats((*this->m_floatData)[entity]);
    return floats[propName];
    }
  static FloatList dummy;
  return dummy;
}

smtk::model::FloatList& Manager::floatProperty(
  const UUID& entity, const std::string& propName)
{
  if (!entity.isNull())
    {
    FloatData& floats((*this->m_floatData)[entity]);
    return floats[propName];
    }
  static FloatList dummy;
  return dummy;
}

bool Manager::hasFloatProperty(
  const UUID& entity, const std::string& propName) const
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

bool Manager::removeFloatProperty(
  const UUID& entity,
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
  if (uit->second.empty())
    this->m_floatData->erase(uit);
  return true;
}

const UUIDWithFloatProperties Manager::floatPropertiesForEntity(const UUID& entity) const
{
  return this->m_floatData->find(entity);
}

UUIDWithFloatProperties Manager::floatPropertiesForEntity(const UUID& entity)
{
  return this->m_floatData->find(entity);
}

void Manager::setStringProperty(
  const UUID& entity,
  const std::string& propName,
  const smtk::model::String& propValue)
{
  smtk::model::StringList tmp;
  tmp.push_back(propValue);
  this->setStringProperty(entity, propName, tmp);
}

void Manager::setStringProperty(
  const UUID& entity,
  const std::string& propName,
  const smtk::model::StringList& propValue)
{
  if (!entity.isNull())
    {
    (*this->m_stringData)[entity][propName] = propValue;
    }
}

smtk::model::StringList const& Manager::stringProperty(
  const UUID& entity, const std::string& propName) const
{
  if (!entity.isNull())
    {
    StringData& strings((*this->m_stringData)[entity]);
    return strings[propName];
    }
  static StringList dummy;
  return dummy;
}

smtk::model::StringList& Manager::stringProperty(
  const UUID& entity, const std::string& propName)
{
  if (!entity.isNull())
    {
    StringData& strings((*this->m_stringData)[entity]);
    return strings[propName];
    }
  static StringList dummy;
  return dummy;
}

bool Manager::hasStringProperty(
  const UUID& entity, const std::string& propName) const
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

bool Manager::removeStringProperty(
  const UUID& entity,
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
  if (uit->second.empty())
    this->m_stringData->erase(uit);
  return true;
}

const UUIDWithStringProperties Manager::stringPropertiesForEntity(const UUID& entity) const
{
  return this->m_stringData->find(entity);
}

UUIDWithStringProperties Manager::stringPropertiesForEntity(const UUID& entity)
{
  return this->m_stringData->find(entity);
}

void Manager::setIntegerProperty(
  const UUID& entity,
  const std::string& propName,
  smtk::model::Integer propValue)
{
  smtk::model::IntegerList tmp;
  tmp.push_back(propValue);
  this->setIntegerProperty(entity, propName, tmp);
}

void Manager::setIntegerProperty(
  const UUID& entity,
  const std::string& propName,
  const smtk::model::IntegerList& propValue)
{
  if (!entity.isNull())
    {
    (*this->m_integerData)[entity][propName] = propValue;
    }
}

smtk::model::IntegerList const& Manager::integerProperty(
  const UUID& entity, const std::string& propName) const
{
  if (!entity.isNull())
    {
    IntegerData& integers((*this->m_integerData)[entity]);
    return integers[propName];
    }
  static IntegerList dummy;
  return dummy;
}

smtk::model::IntegerList& Manager::integerProperty(
  const UUID& entity, const std::string& propName)
{
  if (!entity.isNull())
    {
    IntegerData& integers((*this->m_integerData)[entity]);
    return integers[propName];
    }
  static IntegerList dummy;
  return dummy;
}

bool Manager::hasIntegerProperty(
  const UUID& entity, const std::string& propName) const
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

bool Manager::removeIntegerProperty(
  const UUID& entity,
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
  if (uit->second.empty())
    this->m_integerData->erase(uit);
  return true;
}

const UUIDWithIntegerProperties Manager::integerPropertiesForEntity(const UUID& entity) const
{
  return this->m_integerData->find(entity);
}

UUIDWithIntegerProperties Manager::integerPropertiesForEntity(const UUID& entity)
{
  return this->m_integerData->find(entity);
}
///@}

/// Attempt to find a model owning the given entity.
UUID Manager::modelOwningEntity(const UUID& ent) const
{
  UUID uid(ent);
  UUIDsToEntities::const_iterator it = this->m_topology->find(uid);
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
        UUIDArray::const_iterator sit = it->second.relations().begin();
        sit != it->second.relations().end();
        ++sit)
        {
        UUIDsToEntities::const_iterator subentity = this->topology().find(*sit);
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
        UUIDArray::const_iterator sit = it->second.relations().begin();
        sit != it->second.relations().end();
        ++sit)
        {
        UUIDsToEntities::const_iterator subentity = this->topology().find(*sit);
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
        UUIDArray::const_iterator sit = it->second.relations().begin();
        sit != it->second.relations().end();
        ++sit)
        {
        UUIDsToEntities::const_iterator subentity = this->topology().find(*sit);
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
        UUIDArray::const_iterator sit = it->second.relations().begin();
        sit != it->second.relations().end();
        ++sit)
        {
        UUIDsToEntities::const_iterator subentity = this->topology().find(*sit);
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
      // we attempt to cast ourselves to Manager and identify a parent model.
        {
        // Although const_pointer_cast is evil, changing the entityref classes
        // to accept any type of shared_ptr<X/X const> is more evil.
        ManagerPtr self = smtk::const_pointer_cast<Manager>(shared_from_this());
        ModelEntities parents;
        EntityRefArrangementOps::appendAllRelations(Model(self,ent), EMBEDDED_IN, parents);
        if (!parents.empty())
          return parents[0].entity();
        return UUID::null();
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
    UUIDs uids;
    uids.insert(it->first);
    for (dim = it->second.dimension(); dim >= 0 && dim < 4; ++dim)
      {
      for (UUIDs::iterator uit = uids.begin(); uit != uids.end(); ++uit)
        {
        const Entity* bordEnt = this->findEntity(*uit);
        if (!bordEnt) continue;
        for (UUIDArray::const_iterator rit = bordEnt->relations().begin(); rit != bordEnt->relations().end(); ++rit)
          {
          const Entity* relEnt = this->findEntity(*rit);
          if (relEnt && relEnt != bordEnt && (relEnt->entityFlags() & MODEL_ENTITY))
            {
            return *rit;
            }
          }
        }
      // FIXME: This is slow. Avoid calling bordantEntities().
      uids = this->bordantEntities(uids, dim + 1);
      }
    }
  return UUID::null();
}

/**\brief Return a session associated with the given model.
  *
  * Because modeling operations require access to the un-transcribed model
  * and the original modeling kernel, operations are associated with the
  * session that performs the transcription.
  *
  * \sa Session
  */
SessionPtr Manager::sessionForModel(const UUID& uid) const
{
  // See if the passed entity has a session.
  UUIDsToSessions::const_iterator it = this->m_modelSessions.find(uid);
  if (it != this->m_modelSessions.end())
    return it->second;

  // Nope? OK, see if we can go up a tree of models to find a
  // parent that does have a session.
  UUID entry(uid);
  while (
    (entry = this->modelOwningEntity(entry)) &&
    ((it = this->m_modelSessions.find(entry)) == this->m_modelSessions.end()))
    /* keep trying */
    ;
  if (it != this->m_modelSessions.end())
    return it->second;

  // Nope? Return the default session.
  if (!this->m_defaultSession)
    {
    Manager* self = const_cast<Manager*>(this);
    self->m_defaultSession = smtk::model::DefaultSession::create();
    self->registerSession(self->m_defaultSession);
    }
  return this->m_defaultSession;
}

/**\brief Associate a session with the given model.
  *
  * The \a uid and all its children (excepting those which have their
  * own session set) will be associated with the given \a session.
  * If \a uid already had a session entry, it will be changed to the
  * specified \a session.
  *
  * \sa Session
  */
void Manager::setSessionForModel(
  SessionPtr session, const UUID& uid)
{
  this->m_modelSessions[uid] = session;
}

/**\brief Assign a string property named "name" to every entity without one.
  *
  * This descends sessions and models owned by sessions rather than
  * blindly iterating over UUIDs; it is thus much faster than calling
  * assignDefaultName() on each entity UUID.
  */
void Manager::assignDefaultNames()
{
  // I. Put every UUID into a bin for processing
  UUIDWithEntity it;
  UUIDs models; // models that have not had all of their children named
  UUIDs orphans; // entities that may or may not be parent-less
  UUIDs named; // entities with names
  for (it = this->m_topology->begin(); it != this->m_topology->end(); ++it)
    {
    BitFlags etype = it->second.entityFlags();
    if (etype & MODEL_ENTITY)
      {
      models.insert(it->first);
      }
    else
      {
      if (this->hasStringProperty(it->first, "name"))
        named.insert(it->first);
      else
        orphans.insert(it->first);
      }
    }
  UUIDs::iterator uit;
  for (uit = models.begin(); uit != models.end(); ++uit)
    {
    // Assign the owner a name if required. This way,
    // assignDefaultNamesWithOwner can assume the name exists.
    std::string oname;
    if (!this->hasStringProperty(*uit, "name"))
      oname = this->assignDefaultName(*uit);
    else
      oname = this->stringProperty(*uit, "name")[0];

    UUIDWithEntity iit = this->m_topology->find(*uit);
    this->assignDefaultNamesWithOwner(iit, *uit, oname, orphans, false);
    }
  for (uit = orphans.begin(); uit != orphans.end(); ++uit)
    {
    this->assignDefaultName(*uit);
    }
}

/**\brief Assign a string property named "name" to the given entity.
  *
  * If a model can be identified as owning an entity, the default name
  * assigned to the entity will be the model's name followed by a comma
  * and then the name for the entity. The model's per-entity-type counters
  * are used to number entities of the same type (e.g., "Face 13", "Edge 42").
  *
  * Orphan entities (those without owning models) are given names
  * that end with the trailing digits of their UUIDs.
  */
std::string Manager::assignDefaultName(const UUID& uid)
{
  UUIDWithEntity it = this->m_topology->find(uid);
  if (it != this->m_topology->end())
    {
    return this->assignDefaultName(it->first, it->second.entityFlags());
    }
  return std::string();
}

void Manager::assignDefaultNamesWithOwner(
  const UUIDWithEntity& irec,
  const UUID& owner,
  const std::string& ownersName,
  std::set<smtk::common::UUID>& remaining,
  bool nokids)
{
  remaining.erase(irec->first);
  // Assign the item a name if required:
  if (!this->hasStringProperty(irec->first, "name"))
    {
    IntegerList& counts(this->entityCounts(owner, irec->second.entityFlags()));
    std::string defaultName =
      counts.empty() ?
      this->shortUUIDName(irec->first, irec->second.entityFlags()) :
      ownersName + ", " + Entity::defaultNameFromCounters(irec->second.entityFlags(), counts);
    this->setStringProperty(irec->first, "name", defaultName);
    }

  if (nokids)
    return;

  // Now descend the owner and assign its children names.
  // Do not ascend... check that relIt dimension decreases or
  // that certain ownership rules are met.
  UUIDArray::const_iterator relIt;
  BitFlags iflg = irec->second.entityFlags();
  BitFlags idim = iflg & ANY_DIMENSION;
  for (relIt = irec->second.relations().begin(); relIt != irec->second.relations().end(); ++relIt)
    {
    UUIDWithEntity child = this->m_topology->find(*relIt);
    if (child == this->m_topology->end())
      continue;
    BitFlags cflg = child->second.entityFlags();
    bool yesButNoKids = (cflg & GROUP_ENTITY) && (iflg & MODEL_ENTITY);
    if (
      ((cflg & ANY_DIMENSION) < idim && !(iflg & SHELL_ENTITY)) ||
      ((cflg & SHELL_ENTITY) && (iflg & USE_ENTITY)) ||
      ((cflg & USE_ENTITY)   && (iflg & CELL_ENTITY)) ||
      yesButNoKids)
      {
      this->assignDefaultNamesWithOwner(child, owner, ownersName, remaining, yesButNoKids);
      }
    }
}

std::string Manager::assignDefaultName(const UUID& uid, BitFlags entityFlags)
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
      int count = this->m_globalCounters[1]++;
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
  else if (entityFlags & SESSION)
    {
    std::string tmpName;
    if (!this->hasStringProperty(uid,"name"))
      {
      tmpName =
        Entity::defaultNameFromCounters(
          entityFlags, this->m_globalCounters);
      this->setStringProperty(uid, "name", tmpName);
      }
    else
      tmpName = this->stringProperty(uid, "name")[0];
    return tmpName;
    }
  // Otherwise, use the "owning" model as part of the default name
  // for the entity. First, get the name of the entity's owner:
  UUID owner(
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

std::string Manager::shortUUIDName(const UUID& uid, BitFlags entityFlags)
{
  std::string name = Entity::flagSummaryHelper(entityFlags);
  name += "..";
  std::string uidStr = uid.toString();
  name += uidStr.substr(uidStr.size() - 4);
  return name;
}

/// Return a list of the names of each session subclass whose constructor has been registered with SMTK.
StringList Manager::sessionTypeNames()
{
  return SessionRegistrar::sessionTypeNames();
}

/// Return the list of file types this session can read (currently: a list of file extensions).
StringData Manager::sessionFileTypes(const std::string& bname, const std::string& engine)
{
  return SessionRegistrar::sessionFileTypes(bname, engine);
}

/**\brief Create a session given the type of session to construct.
  *
  * \warning This creates a Session instance without a matching Entity record!
  *          use this method with care; you most probably want createSession
  *          instead.
  */
SessionPtr Manager::createSessionOfType(const std::string& bname)
{
  return SessionRegistrar::createSession(bname);
}

/**\brief Convenience method to create a session without specifying a session ID.
  *
  */
SessionRef Manager::createSession(const std::string& bname)
{
  return this->createSession(bname, SessionRef());
}

/**\brief Create a session, optionally forcing a session ID and/or
  *       registering it with this manager instance.
  *
  */
SessionRef Manager::createSession(
  const std::string& bname,
  const smtk::model::SessionRef& sessionIdSpec)
{
  if (sessionIdSpec.isValid() && this->sessionData(sessionIdSpec))
    return sessionIdSpec; // Hrm, the specified session already exists...

  SessionPtr result = Manager::createSessionOfType(bname);
  if (result)
    {
    if (!sessionIdSpec.entity().isNull())
      result->setSessionId(sessionIdSpec.entity());
    return this->registerSession(result); // will call result->setManager(this);
    }

  smtkInfoMacro(this->m_log, "Could not create \"" << bname << "\" session.");
  return SessionRef();
}

/// Mark the start of a modeling session by registering the \a session with SMTK backing storage.
SessionRef Manager::registerSession(SessionPtr session)
{
  if (!session)
    return SessionRef();

  UUID sessId = session->sessionId();
  if (sessId.isNull())
    return SessionRef();

  (*this->m_sessions)[sessId] = session;
  Manager::iter_type brec =
    this->setEntityOfTypeAndDimension(sessId, SESSION, -1);

  session->setManager(this);
  return SessionRef(shared_from_this(), brec->first);
}

/**\brief Mark the end of a modeling session by removing its \a session.
  *
  * This will remove session-member entities if \a expungeSession is true.
  */
bool Manager::unregisterSession(SessionPtr session, bool expungeSession)
{
  if (!session)
    return false;

  UUID sessId = session->sessionId();
  if (sessId.isNull())
    return false;

  if (expungeSession)
    { // Carefully remove all children of session (models, volumes, faces, etc.)
    this->erase(sessId);
    }
  else
    {
    // Remove the session's entity record, properties, and such, but not
    // records, properties, etc. for entities the session owns.
    this->m_topology->erase(sessId);
    this->m_floatData->erase(sessId);
    this->m_stringData->erase(sessId);
    this->m_integerData->erase(sessId);
    this->m_arrangements->erase(sessId);
    this->m_tessellations->erase(sessId);
    this->m_attributeAssignments->erase(sessId);
    }
  return this->m_sessions->erase(sessId) ? true : false;
}

/// Find a session given its session UUID (or NULL).
SessionPtr Manager::sessionData(const smtk::model::SessionRef& sessId) const
{
  if (sessId.entity().isNull())
    return this->m_defaultSession;

  UUIDsToSessions::const_iterator it = this->m_sessions->find(sessId.entity());
  if (it == this->m_sessions->end())
    return SessionPtr();
  return it->second;
}

/// Return a reference to the \a modelId's counter array associated with the given \a entityFlags.
IntegerList& Manager::entityCounts(
  const UUID& modelId, BitFlags entityFlags)
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
void Manager::prepareForEntity(std::pair<UUID,Entity>& entry)
{
  if ((entry.second.entityFlags() & CELL_2D) == CELL_2D)
    {
    if (!this->hasArrangementsOfKindForEntity(entry.first, HAS_USE))
      {
      // Create arrangements to hold face-uses:
      this->arrangeEntity(entry.first, HAS_USE, Arrangement::CellHasUseWithIndexSenseAndOrientation(-1, 0, NEGATIVE));
      this->arrangeEntity(entry.first, HAS_USE, Arrangement::CellHasUseWithIndexSenseAndOrientation(-1, 1, POSITIVE));
      }
    }
  else if (entry.second.entityFlags() & USE_ENTITY)
    {
    if (!this->hasArrangementsOfKindForEntity(entry.first, HAS_SHELL))
      {
      // Create arrangement to hold parent shell:
      this->arrangeEntity(entry.first, HAS_SHELL, Arrangement::UseHasShellWithIndex(-1));
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

/**@name Find entities by their property values.
  *\brief Look for entities that have a given property defined and whose value matches one provided.
  *
  * The non-templated variants returning EntityRefArray can be wrapped and used in Python
  * while the templated variants are more useful in C++.
  */
//@{
/// Find entities with an integer property named \a pname whose value is the single value \a pval.
EntityRefArray Manager::findEntitiesByProperty(const std::string& pname, Integer pval)
{
  return this->findEntitiesByPropertyAs<EntityRefArray>(pname, pval);
}

/// Find entities with a floating-point property named \a pname whose value is the single value \a pval.
EntityRefArray Manager::findEntitiesByProperty(const std::string& pname, Float pval)
{
  return this->findEntitiesByPropertyAs<EntityRefArray>(pname, pval);
}

/// Find entities with a string property named \a pname whose value is the single value \a pval.
EntityRefArray Manager::findEntitiesByProperty(const std::string& pname, const std::string& pval)
{
  return this->findEntitiesByPropertyAs<EntityRefArray>(pname, pval);
}

/// Find entities with an integer property named \a pname whose every value matches the array \a pval.
EntityRefArray Manager::findEntitiesByProperty(const std::string& pname, const IntegerList& pval)
{
  return this->findEntitiesByPropertyAs<EntityRefArray>(pname, pval);
}

/// Find entities with a floating-point property named \a pname whose every value matches the array \a pval.
EntityRefArray Manager::findEntitiesByProperty(const std::string& pname, const FloatList& pval)
{
  return this->findEntitiesByPropertyAs<EntityRefArray>(pname, pval);
}

/// Find entities with a string property named \a pname whose every value matches the array \a pval.
EntityRefArray Manager::findEntitiesByProperty(const std::string& pname, const StringList& pval)
{
  return this->findEntitiesByPropertyAs<EntityRefArray>(pname, pval);
}

/*! \fn template<typename Collection> Collection Manager::findEntitiesByPropertyAs(const std::string& pname, Integer pval)
 * \brief Return entities with a property named \a pname whose values match the given \a pval.
 */

/*! \fn template<typename Collection> Collection Manager::findEntitiesByPropertyAs(const std::string& pname, const IntegerList& pval)
 * \brief Return entities with a property named \a pname whose values match the given \a pval.
 */

/*! \fn template<typename Collection> Collection Manager::findEntitiesByPropertyAs(const std::string& pname, Float pval)
 * \brief Return entities with a property named \a pname whose values match the given \a pval.
 */

/*! \fn template<typename Collection> Collection Manager::findEntitiesByPropertyAs(const std::string& pname, const FloatList& pval)
 * \brief Return entities with a property named \a pname whose values match the given \a pval.
 */

/*! \fn template<typename Collection> Collection Manager::findEntitiesByPropertyAs(const std::string& pname, const std::string& pval)
 * \brief Return entities with a property named \a pname whose values match the given \a pval.
 */

/*! \fn template<typename Collection> Collection Manager::findEntitiesByPropertyAs(const std::string& pname, const StringList& pval)
 * \brief Return entities with a property named \a pname whose values match the given \a pval.
 */
//@}

/**\brief Find entities whose type matches the given \a flags.
  *
  * This version can be wrapped and used in Python.
  * It is not named entitiesMatchingFlags (to mirror the
  * templated entitiesMatchingFlagsAs<T>) because our base
  * class, Manager, provides another method of the same
  * name that returns UUIDs rather than EntityRefArray.
  */
EntityRefArray Manager::findEntitiesOfType(BitFlags flags, bool exactMatch)
{
  return this->entitiesMatchingFlagsAs<EntityRefArray>(flags, exactMatch);
}

/// Set the tessellation information for a given \a cellId.
Manager::tess_iter_type Manager::setTessellation(const UUID& cellId, const Tessellation& geom)
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

/**\brief Add or replace information about the arrangement of an entity.
  *
  * When \a index is -1, the arrangement is considered new and added to the end of
  * the vector of arrangements of the given \a kind.
  * Otherwise, it should be positive and refer to a pre-existing arrangement to be replaced.
  * The actual \a index location used is returned.
  */
int Manager::arrangeEntity(const UUID& entityId, ArrangementKind kind, const Arrangement& arr, int index)
{
  UUIDsToArrangements::iterator cit = this->m_arrangements->find(entityId);
  if (cit == this->m_arrangements->end())
    {
    if (index >= 0)
      { // failure: can't replace information that doesn't exist.
      return -1;
      }
    KindsToArrangements blank;
    cit = this->m_arrangements->insert(std::pair<UUID,KindsToArrangements>(entityId, blank)).first;
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

/**\brief Remove an arrangement from an entity, and optionally the entity itself.
  *
  * When no action is taken (because of a missing entityId, a missing arrangement
  * or a bad index), 0 is returned.
  * When a the arrangement is successfully removed, 1 is returned.
  * When \a removeIfLast is true and the entity is removed, 2 is returned.
  * When the related entity specified by the arrangement is also removed (only
  * attempted when \a removeIfLast is true), 3 is returned.
  *
  * **Warning**: Invoking this method naively will likely result
  * in an inconsistent solid model.
  * The caller is expected to perform further operations to keep
  * the model valid.
  */
int Manager::unarrangeEntity(const UUID& entityId, ArrangementKind k, int index, bool removeIfLast)
{
  int result = 0;
  bool canRemoveEntity = false;
  if (index < 0)
    return result;
  UUIDWithArrangementDictionary ad = this->m_arrangements->find(entityId);
  if (ad == this->m_arrangements->end())
   return result;
  ArrangementKindWithArrangements ak = ad->second.find(k);
  if (ak == ad->second.end() || index >= static_cast<int>(ak->second.size()))
    return result;

  // TODO: notify relation + entity (or their delegates) of imminent removal?
  ak->second.erase(ak->second.begin() + index);
  ++result;

  // Now, if we removed the last arrangement of this kind, kill the kind-dictionary entry
  if (ak->second.empty())
    {
    ad->second.erase(ak);
    // Now if we removed the last kind with arrangements in ad, kill the uuid-dictionary entry
    if (ad->second.empty())
      {
      this->m_arrangements->erase(ad);
      canRemoveEntity = true;
      }
    }

  // Now find and remove the dual arrangement (if one exists)
  // This branch should not be taken if we are inside the inner unarrangeEntity call below.
  ArrangementReferences duals;
  if (this->findDualArrangements(entityId, k, index, duals))
    {
    // Unarrange every dual to this arrangement.
    bool canIncrement = false;
    for (ArrangementReferences::iterator dit = duals.begin(); dit != duals.end(); ++dit)
      {
      if (this->unarrangeEntity(dit->entityId, dit->kind, dit->index, removeIfLast) == 2)
        canIncrement = true; // Only increment result when dualEntity is remove, not the dual arrangement.
      }
    // Only increment once if other entities were removed; we do not indicate how many were removed.
    if (canIncrement) ++result;
    }

  // If we removed the last arrangement relating this entity to others,
  // and if the caller has requested it: remove the entity itself.
  if (removeIfLast && canRemoveEntity)
    {
    // TODO: notify entity of removal.
    this->m_topology->erase(entityId);
    ++result;
    }
  return result;
}

/**\brief Returns true when the given \a entity has any arrangements of the given \a kind (otherwise false).
  *
  * Use this to avoid accidentally inserting a new array of arrangements with arrangementsOfKindForEntity().
  * Since this actually requires a lookup, you may pass in a pointer \a arr to an array of arrangements;
  * if true is returned, the pointer will be aimed at the existing array. Otherwise, \a arr will be unchanged.
  */
Arrangements* Manager::hasArrangementsOfKindForEntity(
  const UUID& entity, ArrangementKind kind)
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
const Arrangements* Manager::hasArrangementsOfKindForEntity(
  const UUID& entity, ArrangementKind kind) const
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
Arrangements& Manager::arrangementsOfKindForEntity(
  const UUID& entity,
  ArrangementKind kind)
{
  return (*this->m_arrangements)[entity][kind];
}

/**\brief Retrieve arrangement information for a cell.
  *
  * This version does not allow the arrangement to be altered.
  */
const Arrangement* Manager::findArrangement(const UUID& cellId, ArrangementKind kind, int index) const
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
Arrangement* Manager::findArrangement(const UUID& cellId, ArrangementKind kind, int index)
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

/**\brief Find an arrangement of type \a kind that relates \a entityId to \a involvedEntity.
  *
  * This method returns the index upon success and a negative number upon failure.
  */
int Manager::findArrangementInvolvingEntity(
  const UUID& entityId, ArrangementKind kind,
  const UUID& involvedEntity) const
{
  const Entity* src = this->findEntity(entityId);
  if (!src)
    return -1;

  const Arrangements* arr = this->hasArrangementsOfKindForEntity(entityId, kind);
  if (!arr)
    return -1;

  Arrangements::const_iterator it;
  int idx = 0;
  UUIDArray rels;
  for (it = arr->begin(); it != arr->end(); ++it, ++idx, rels.clear())
    if (it->relations(rels, src, kind))
      for (UUIDArray::iterator rit = rels.begin(); rit != rels.end(); ++rit)
        if (*rit == involvedEntity)
          return idx;

  return -1;
}

/**\brief Find the inverse of the given arrangement, if it exists.
  *
  * When an arrangement relates one entity to another, it usually
  * has a dual, or inverse, arrangement stored with the other entity
  * so that the relationship may be discovered given either of the
  * entities involved.
  *
  * Note that the dual is not related to sense or orientation;
  * for example the dual of a face-cell's HAS_USE arrangement is
  * *not* the opposite face. Rather, the dual is the face-use
  * record's HAS_CELL arrangement (pointing from the face-use to
  * the face-cell).
  *
  * Because some relations are one-to-many in nature, it is possible
  * for the dual of a relation to have multiple values. For example,
  * a Shell's HAS_USE arrangement refer to many FaceUse instances.
  * For this reason, findDualArrangements returns an array of duals.
  *
  * This method and smtk::model::Arrangement::relations() are
  * the two main methods which determine how arrangements should be
  * interpreted in context without any prior constraints on the
  * context. (Other methods create and interpret arrangements in
  * specific circumstances where the context is known.)
  */
bool Manager::findDualArrangements(
    const UUID& entityId, ArrangementKind kind, int index,
    ArrangementReferences& duals) const
{
  if (index < 0)
    return false;

  const Entity* src = this->findEntity(entityId);
  if (!src)
    return false;

  const Arrangements* arr = this->hasArrangementsOfKindForEntity(entityId, kind);
  if (!arr || index >= static_cast<int>(arr->size()))
    return false;

  int relationIdx;
  int sense;
  Orientation orient;

  UUID dualEntityId;
  ArrangementKind dualKind;
  int dualIndex;
  int relStart, relEnd;

  switch (kind)
    {
  case HAS_USE:
    switch (src->entityFlags() & ENTITY_MASK)
      {
    case CELL_ENTITY:
      if ((*arr)[index].IndexSenseAndOrientationFromCellHasUse(relationIdx, sense, orient))
        { // OK, find use's reference to this cell.
        dualEntityId = src->relations()[relationIdx];
        dualKind = HAS_CELL;
        if ((dualIndex =
            this->findArrangementInvolvingEntity(
              dualEntityId, dualKind, entityId)) >= 0)
          {
          duals.push_back(ArrangementReference(dualEntityId, dualKind, dualIndex));
          return true;
          }
        }
       break;
    case SHELL_ENTITY:
      if ((*arr)[index].IndexRangeFromShellHasUse(relStart, relEnd))
        { // Find the use's reference to this shell.
        dualKind = HAS_SHELL;
        for (; relStart != relEnd; ++relStart)
          {
          dualEntityId = src->relations()[relStart];
          if ((dualIndex =
              this->findArrangementInvolvingEntity(
                dualEntityId, dualKind, entityId)) >= 0)
            duals.push_back(ArrangementReference(dualEntityId, dualKind, dualIndex));
          }
        if (!duals.empty()) return true;
        }
      break;
      /*
      bool IndexFromCellEmbeddedInEntity(int& relationIdx) const;
      bool IndexFromCellIncludesEntity(int& relationIdx) const;
      bool IndexFromCellHasShell(int& relationIdx) const;
      bool IndexAndSenseFromUseHasCell(int& relationIdx, int& sense) const;
      bool IndexFromUseHasShell(int& relationIdx) const;
      bool IndexFromUseOrShellIncludesShell(int& relationIdx) const;
      bool IndexFromShellHasCell(int& relationIdx) const;
      bool IndexRangeFromShellHasUse(int& relationBegin, int& relationEnd) const;
      bool IndexFromShellEmbeddedInUseOrShell(int& relationIdx) const;
      bool IndexFromInstanceInstanceOf(int& relationIdx) const;
      bool IndexFromEntityInstancedBy(int& relationIdx) const;
      */
    case USE_ENTITY:
    case GROUP_ENTITY:
    case MODEL_ENTITY:
    case INSTANCE_ENTITY:
      break;
      }
  case HAS_CELL:
  case HAS_SHELL:
  case INCLUDES:
  case EMBEDDED_IN:
  case SUPERSET_OF:
  case SUBSET_OF:
  case INSTANCE_OF:
  case INSTANCED_BY:
  default:
    break;
    }
  return false;
}

/**\brief Find a particular arrangement: a cell's HAS_USE with a given sense.
  *
  * The index of the matching arrangement is returned (or -1 if no such sense
  * exists).
  *
  * The sense is a non-negative integer corresponding to a particular
  * use of a cell. However, the model may be altered in a way that some
  * senses are no longer used.
  * Rather than rewrite the cell and cell-use records to keep senses
  * as a continuous integer sequence, we allow "holes" to exist in the
  * list of senses. Just because a cell has a use with sense 5 does not
  * imply that there is also a use with sense 4.
  *
  * You may find all the HAS_USE arrangements of the cell and iterator over
  * them to discover all the sense numbers.
  * There should be no duplicate senses for any given cell.
  */
int Manager::findCellHasUseWithSense(
  const UUID& cellId, int sense) const
{
  const Arrangements* arrs = this->hasArrangementsOfKindForEntity(cellId, HAS_USE);
  if (arrs)
    {
    int i = 0;
    for (Arrangements::const_iterator it = arrs->begin(); it != arrs->end(); ++it, ++i)
      {
      int itIdx, itSense;
      Orientation itOrient;
      if (
        it->IndexSenseAndOrientationFromCellHasUse(itIdx, itSense, itOrient) &&
        itSense == sense)
        {
        return i;
        }
      }
    }
  return -1;
}

/**\brief Find HAS_USE arrangements of a cell with a given orientation.
  *
  * The indices of the matching arrangements are returned.
  */
std::set<int> Manager::findCellHasUsesWithOrientation(
  const UUID& cellId, Orientation orient) const
{
  std::set<int> result;
  const Arrangements* arrs = this->hasArrangementsOfKindForEntity(cellId, HAS_USE);
  if (arrs)
    {
    int i = 0;
    for (Arrangements::const_iterator it = arrs->begin(); it != arrs->end(); ++it, ++i)
      {
      int itIdx, itSense;
      Orientation itOrient;
      if (
        it->IndexSenseAndOrientationFromCellHasUse(itIdx, itSense, itOrient) &&
        itOrient == orient)
        {
        result.insert(i);
        }
      }
    }
  return result;
}

/**\brief Return the UUID of a use record for the
  * given \a cell and \a sense, or NULL if it does not exist.
  */
UUID Manager::cellHasUseOfSenseAndOrientation(
  const UUID& cell, int sense, Orientation orient) const
{
  const smtk::model::Arrangements* arr;
  if ((arr = this->hasArrangementsOfKindForEntity(cell, HAS_USE)) && !arr->empty())
    { // See if any of this cell's uses match our sense.
    for (smtk::model::Arrangements::const_iterator ait = arr->begin(); ait != arr->end(); ++ait)
      {
      int itIdx;
      int itSense;
      Orientation itOrient;
      ait->IndexSenseAndOrientationFromCellHasUse(itIdx, itSense, itOrient);
      if (itSense == sense && itOrient == orient)
        {
        return this->findEntity(cell)->relations()[itIdx];
        }
      }
    }
  return UUID::null();
}

/**\brief Find a use record for the given \a cell and \a sense,
  * creating one if it does not exist or replacing it if \a replacement is non-NULL.
  *
  */
UUID Manager::findCreateOrReplaceCellUseOfSenseAndOrientation(
  const UUID& cell, int sense, Orientation orient,
  const UUID& replacement)
{
  Entity* entity = this->findEntity(cell);
  if (!entity)
    {
    return UUID::null();
    }
  smtk::model::Arrangements& arr(
    this->arrangementsOfKindForEntity(cell, HAS_USE));

  // See if any of this cell's uses match our sense...
  int arrIdx = -1;
  smtk::model::Arrangements::const_iterator ait;
  int relIdx = 0;
  for (ait = arr.begin(); ait != arr.end(); ++ait, ++relIdx)
    {
    int itIdx;
    int itSense;
    Orientation itOrient;
    ait->IndexSenseAndOrientationFromCellHasUse(itIdx, itSense, itOrient);
    if (itSense == sense && itOrient == orient)
      {
      if (itIdx >= 0)
        { // Found a valid use. If we have a replacement, use it.
        if (!replacement.isNull())
          {
          this->unarrangeEntity(cell, HAS_USE, relIdx, true);
          arrIdx = relIdx;
          break;
          }
        return entity->relations()[itIdx];
        }
      else
        { // We found an existing but invalid use... replace it below.
        arrIdx = relIdx;
        break;
        }
      }
    }

  // ...nope, we need to create a new use with
  // the specified sense relative to the cell.
  // Note that there may still be an entry in arr
  // which we should overwrite (with itIdx == -1).
  UUIDWithEntity use;
  if (replacement.isNull())
    {
    use = this->insertEntityOfTypeAndDimension(
      USE_ENTITY | entity->dimensionBits(), entity->dimension());
    // We must re-fetch entity since inserting the use
    // may have invalidated our reference to it.
    entity = this->findEntity(cell);
    }
  else
    {
    use = this->m_topology->find(replacement);
    }

  // Now add the use to the cell and the cell to the use:
  this->arrangeEntity(
    cell, HAS_USE,
    Arrangement::CellHasUseWithIndexSenseAndOrientation(
      entity->appendRelation(use->first), sense, orient),
    arrIdx);
  this->arrangeEntity(
    use->first, HAS_CELL,
    Arrangement::UseHasCellWithIndexAndSense(
      use->second.appendRelation(cell), sense));

  return use->first;
}

/**\brief Return the UUIDs of all shells included by the given cell-use or shell.
  *
  * Cell-uses of dimension d may include shells that span dimensions d and (d-1).
  * Shells may include other shells of the same dimensionality.
  * These relationships define a hierarchy that enumerate the oriented boundary of
  * the top-level cell-use.
  */
UUIDs Manager::useOrShellIncludesShells(
  const UUID& cellUseOrShell) const
{
  UUIDs shells;
  const Entity* ent = this->findEntity(cellUseOrShell);
  if (ent && (ent->entityFlags() & (USE_ENTITY | SHELL_ENTITY)))
    {
    const UUIDArray& rels(ent->relations());
    const smtk::model::Arrangements* arr;
    if ((arr = this->hasArrangementsOfKindForEntity(cellUseOrShell, INCLUDES)) && !arr->empty())
      {
      for (smtk::model::Arrangements::const_iterator ait = arr->begin(); ait != arr->end(); ++ait)
        {
        int itIdx;
        ait->IndexFromUseOrShellIncludesShell(itIdx);
        // Only insert if the inclusion is a shell
        if ((ent = this->findEntity(rels[itIdx])) && (ent->entityFlags() & SHELL_ENTITY))
          {
          shells.insert(rels[itIdx]);
          }
        }
      }
    }
  return shells;
}

/**\brief Add a new shell to the specified \a useOrShell entity as an inclusion.
  *
  * Cell-uses *include* shells relating the use to a grouping of lower-dimensional uses.
  * Each shell included directly by a cell-use represents a disconnected component
  * of the cell.
  * Shells may also include other shells of the same dimensionality representing voids
  * or material within voids for odd or even depths from the parent cell-use, respectively.
  */
UUID Manager::createIncludedShell(const UUID& useOrShell)
{
  Entity* entity = this->findEntity(useOrShell);
  if (!entity)
    {
    return UUID::null();
    }
  int shellDim =
    (entity->entityFlags() & USE_ENTITY || entity->entityFlags() & CELL_ENTITY) ?
    // k-shells span dimensions (d, d-1), where d = dimension of the cell/cell-use:
    entity->dimensionBits() | (entity->dimensionBits() >> 1) :
    // included k-shell must have same dimension as parent:
    entity->dimensionBits();
  int indexOfNewShell = static_cast<int>(entity->relations().size());
  UUIDWithEntity shell =
    this->insertEntityOfTypeAndDimension(SHELL_ENTITY | shellDim, -1);
  this->arrangeEntity(useOrShell, INCLUDES,
    Arrangement::UseOrShellIncludesShellWithIndex(indexOfNewShell));
  // We must re-find the entity record since insertEntityOfTypeAndDimension
  // invalidates entity when SMTK_HASH_STORAGE is true:
  this->findEntity(useOrShell)->appendRelation(shell->first);
  this->arrangeEntity(shell->first, EMBEDDED_IN,
    Arrangement::ShellEmbeddedInUseOrShellWithIndex(
      static_cast<int>(shell->second.relations().size())));
  shell->second.appendRelation(useOrShell);

  return shell->first;
}

/** Add a shell to \a parentUseOrShell as an inclusion unless it already exists.
  *
  * Returns true when adding the shell was necessary.
  * Returns false if either entity does not exist or the shell was already owned by the parent.
  */
bool Manager::findOrAddIncludedShell(
  const UUID& parentUseOrShell,
  const UUID& shellToInclude)
{
  Entity* parEnt = this->findEntity(parentUseOrShell);
  Entity* shlEnt = this->findEntity(shellToInclude);
  if (!parEnt || !shlEnt)
    {
    return false;
    }

  int indexOfShell = this->findArrangementInvolvingEntity(
    parentUseOrShell, INCLUDES, shellToInclude);
  if (indexOfShell >= 0)
    return false;

  // Didn't find it. Add both forward and inverse relations.
  this->arrangeEntity(parentUseOrShell, INCLUDES,
    Arrangement::UseOrShellIncludesShellWithIndex(
      parEnt->appendRelation(shellToInclude)));
  this->arrangeEntity(shellToInclude, EMBEDDED_IN,
    Arrangement::ShellEmbeddedInUseOrShellWithIndex(
      shlEnt->appendRelation(parentUseOrShell)));

  return true;
}

/**\brief Add a cell-use to a shell if it is not already contained in the shell.
  *
  * Note that cell-uses may have relations to shells of 2 different dimensions.
  * This method should only be called when adding d-dimensional
  * use-records to a shell bridging dimensions (d, d+1).
  * For example, call this method when adding an edge-use to a loop and not
  * when setting the edge-use associated with a chain.
  * Use the findOrAddIncludedShell() method to do the latter.
  *
  * The reason for this is that d-shells must have exactly one parent
  * cell-use (or cell, for d=3), but may have many child cell-uses
  * that are one dimension lower.
  * A different type of relationship (INCLUDES/EMBEDDED_IN vs HAS_USE/HAS_SHELL)
  * is employed depending on the dimension so that the distinction can be made
  * easily.
  */
bool Manager::findOrAddUseToShell(
  const UUID& shell, const UUID& use)
{
  Entity* shellEnt;
  Entity* useEnt;
  // Check that the shell and use are valid and that the use has the proper dimension for the shell.
  if (
    (shellEnt = this->findEntity(shell)) &&
    (useEnt = this->findEntity(use)))
    {
    // Verify that the cell-use uses the lower dimension-bit of the shell.
    // E.g., if the shell spans dimensions 1 and 2, the use must be of dimension 1.
    if ((shellEnt->dimensionBits() & useEnt->dimensionBits()) < shellEnt->dimensionBits())
      {
      // Now, is the use already listed in a HAS_USE relationship?
      int anum = -1;
      int srsize = static_cast<int>(shellEnt->relations().size());
      Arrangements* arr = this->hasArrangementsOfKindForEntity(shell, HAS_USE);
      if (srsize && arr)
        {
        int a = 0;
        for (Arrangements::const_iterator ait = arr->begin(); ait != arr->end(); ++ait, ++a)
          {
          int i0, i1;
          ait->IndexRangeFromShellHasUse(i0, i1);
          for (int ii = i0; ii < i1; ++ii)
            {
            if (shellEnt->relations()[ii] == use)
              return false; // The shell already has the use-record. Do nothing.
            }
          // OK, does this HAS_USE arrangement go to the end of shellEnt->relations()?
          if (i1 == srsize)
            { // Yes, so we can extend it below instead of adding a new HAS_USE.
            anum = a;
            }
          }
        }
      // So far, no existing HAS_USE arrangement contained it;
      // add the use to the shell.
      shellEnt->appendRelation(use);
      if (anum >= 0)
        { // We can "extend" an existing arrangement to include the use.
        int i0, i1;
        (*arr)[anum].IndexRangeFromShellHasUse(i0, i1);
        (*arr)[anum] = Arrangement::ShellHasUseWithIndexRange(i0, i1 + 1);
        }
      else
        { // Create a new arrangement
        this->arrangeEntity(shell, HAS_USE, Arrangement::ShellHasUseWithIndexRange(srsize, srsize + 1));
        }
      // Now set the inverse relationship (the cell-use mapping to its parent shell).
      // Every cell-use must have a HAS_SHELL relation which we overwrite --
      // except in the case of vertex-use where we add a new relation since
      // they may have multiple HAS_SHELL relationships.
      int shellIdx = -1;
      bool previousShell = true; // Was there a previous, valid HAS_SHELL arrangement? Assume so.
      Arrangement* a;
      if ((a = this->findArrangement(use, HAS_SHELL, 0)))
        {
        a->IndexFromUseHasShell(shellIdx);
        if (shellIdx < 0)
          {
          shellIdx = useEnt->findOrAppendRelation(shell);
          *this->findArrangement(use, HAS_SHELL, 0) =
            Arrangement::UseHasShellWithIndex(shellIdx);
          previousShell = false;
          }
        }
      else
        {
        shellIdx = this->arrangeEntity(use, HAS_SHELL, Arrangement::UseHasShellWithIndex(-1));
        previousShell = false;
        }
      // Here is where vertex-uses are handled differently:
      // If we aren't replacing an invalid (-1) index in the arrangement,
      // then we should create a new arrangement instead of overwrite
      // what's at shellIdx in useEnt->relations().
      if (isVertexUse(useEnt->entityFlags()) && previousShell)
        {
        shellIdx = this->arrangeEntity(
          use, HAS_SHELL,
          Arrangement::UseHasShellWithIndex(
            useEnt->findOrAppendRelation(shell)));
        }
      else
        {
        useEnt->relations()[shellIdx] = shell;
        }
      return true;
      }
    // FIXME: Should we throw() when dimension is wrong?
    }
  // garbage-in => garbage-out
  // FIXME: Should we throw() here?
  return false;
}

/**\brief Add an entity to a cell as a geometric inclusion.
  *
  * This attempts to add relationships and arrangements to
  * both the \a cell and \a inclusion that indicate the
  * latter is geometrically interior to the \a cell.
  *
  * Thus, the \a inclusion must have a dimension less-than
  * or equal to the \a cell.
  */
bool Manager::findOrAddInclusionToCellOrModel(
  const UUID& cell, const UUID& inclusion)
{
  Entity* cellEnt;
  Entity* incEnt;
  // Check that the cell and inclusion are valid and that the inclusion has the proper dimension for the cell.
  if (
    (cellEnt = this->findEntity(cell)) &&
    (incEnt = this->findEntity(inclusion)))
    {
    // Verify that the inclusion has a lower-or-equal than the cell.
    // E.g., if the cell has dimension 2, the inclusion must be of dimension 2 or less.
    if (cellEnt->dimensionBits() >= incEnt->dimensionBits())
      {
      // Now, is the inclusion already listed in a INCLUDES relationship?
      int srsize = static_cast<int>(cellEnt->relations().size());
      Arrangements* arr = this->hasArrangementsOfKindForEntity(cell, INCLUDES);
      if (srsize && arr)
        {
        int a = 0;
        for (Arrangements::const_iterator ait = arr->begin(); ait != arr->end(); ++ait, ++a)
          {
          int i;
          ait->IndexFromCellIncludesEntity(i);
          if (i >= 0 && cellEnt->relations()[i] == inclusion)
            {
            return false; // The cell already has the inclusion-record. Do nothing.
            }
          }
        }
      // No existing INCLUDES arrangement contained it;
      // add the inclusion to the cell.
      cellEnt->appendRelation(inclusion);
      this->arrangeEntity(cell, INCLUDES, Arrangement::CellIncludesEntityWithIndex(srsize));
      // Now set the inverse relationship (the cell-inclusion mapping to its parent cell).
      // Every cell-inclusion must have a matching EMBEDDED_IN relation.
      int cellIdx = incEnt->findOrAppendRelation(cell);
      this->arrangeEntity(inclusion, EMBEDDED_IN, Arrangement::CellEmbeddedInEntityWithIndex(cellIdx));
      return true;
      }
    // FIXME: Should we throw() when dimension is wrong?
    }
  // garbage-in => garbage-out
  // FIXME: Should we throw() here?
  return false;
}

/**\brief Add an entity as a subset of a group.
  *
  * Note that no group/partition constraints are enforced.
  * Returns true when the entity was successfully added (or already existed)
  * and false upon failure (such as when \a grp or \a ent are invalid).
  */
bool Manager::findOrAddEntityToGroup(const UUID& grp, const UUID& ent)
{
  Group group(shared_from_this(), grp);
  EntityRef member(shared_from_this(), ent);
  int count = 0;
  if (group.isValid() && member.isValid())
    {
    count = static_cast<int>(group.members<EntityRefs>().count(member));
    if (count == 0)
      {
      EntityRefArrangementOps::findOrAddSimpleRelationship(group, SUPERSET_OF, member);
      EntityRefArrangementOps::findOrAddSimpleRelationship(member, SUBSET_OF, group);
      ++count;
      }
    }
  return count > 0 ? true :false;
}

/**@name Attribute association
  *\brief Associate and disassociate attribute values to entities.
  */
//@{
/**\brief Report whether an entity has been assigned an attribute.
  *
  */
bool Manager::hasAttribute(const UUID&  attribId, const UUID& toEntity)
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
  * This returns true when the attribute association is
  * valid (whether it was previously associated or not)
  * and false otherwise.
  */
bool Manager::associateAttribute(const UUID&  attribId, const UUID& toEntity)
{
  bool allowed = true;
  if (this->m_attributeSystem)
    {
    attribute::AttributePtr att = this->m_attributeSystem->findAttribute(attribId);
    if (!att || !att->associateEntity(toEntity))
      allowed = false;
    }
  if (allowed)
    this->attributeAssignments()[toEntity].associateAttribute(attribId);
  return allowed;
}

/**\brief Unassign an attribute from an entity.
  *
  */
bool Manager::disassociateAttribute(const UUID&  attribId, const UUID& fromEntity, bool reverse)
{
  bool didRemove = false;
  UUIDWithAttributeAssignments ref = this->m_attributeAssignments->find(fromEntity);
  if (ref == this->m_attributeAssignments->end())
    {
    return didRemove;
    }
  if ((didRemove = ref->second.disassociateAttribute(attribId)))
    {
    // If the AttributeAssignments instance is now empty, remove it.
    // (Only do this for std::map storage, as it triggers assertion
    // failures in sparsehash for no discernable reason.)
#ifndef SMTK_HASH_STORAGE
    if (ref->second.attributes().empty())
      {
      this->m_attributeAssignments->erase(ref);
      }
#endif
    // Notify the Attribute of the removal
    if (reverse)
      {
      if (this->m_attributeSystem)
        {
        smtk::attribute::AttributePtr attrib =
          this->m_attributeSystem->findAttribute(attribId);
        // FIXME: Should we check that the manager's refManager
        //        is this Manager instance?
        if (attrib)
          {
          attrib->disassociateEntity(fromEntity, false);
          }
        }
      }
    }
  return didRemove;
}
//@}

/**@name Unbacked entity insertion methods
  *\brief Methods to insert entities into the local storage independent of a session.
  *
  * The methods that start with "add" will generate a UUID for you and return
  * a entityref to the new entity.
  * Methods that start with "insert" accept a UUID and will return a entityref to
  * the new entity (or the existing entity if it matches the entity type being
  * created). If you specify a UUID in use by an entity of a different type, an
  * invalid entityref will be returned.
  * Finally, methods that start with "set" will either modify an existing entity
  * or create a new one as required. The "set" methods are used to modify arrangements
  * that may have been created as part of constructing other entities (e.g., calling
  * addFace() creates arrangements for 2 face-uses; you can then use setFaceUse to
  * replace any existing use records with the one you specify).
  */
//@{
/// Add a vertex to the manager (without any relationships) at the given \a uid.
Vertex Manager::insertVertex(const UUID& uid)
{
  return Vertex(
    shared_from_this(),
    this->setEntityOfTypeAndDimension(uid, CELL_ENTITY, 0)->first);
}

/// Add an edge to the manager (without any relationships) at the given \a uid.
Edge Manager::insertEdge(const UUID& uid)
{
  return Edge(
    shared_from_this(),
    this->setEntityOfTypeAndDimension(uid, CELL_ENTITY, 1)->first);
}

/// Add a face to the manager (without any relationships) at the given \a uid.
Face Manager::insertFace(const UUID& uid)
{
  return Face(
    shared_from_this(),
    this->setEntityOfTypeAndDimension(uid, CELL_ENTITY, 2)->first);
}

/// Add a volume to the manager (without any relationships) at the given \a uid.
Volume Manager::insertVolume(const UUID& uid)
{
  return Volume(
    shared_from_this(),
    this->setEntityOfTypeAndDimension(uid, CELL_ENTITY, 3)->first);
}

/// Add an edge to the manager (without any relationships)
Vertex Manager::addVertex()
{
  return Vertex(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(CELL_ENTITY, 0));
}

/// Add an edge to the manager (without any relationships)
Edge Manager::addEdge()
{
  return Edge(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(CELL_ENTITY, 1));
}

/**\brief Add a face to the manager (without any relationships)
  *
  * While this method does not add any relations, it
  * does create two HAS_USE arrangements to hold
  * FaceUse instances (assuming the Manager may be
  * downcast to a Manager instance).
  */
Face Manager::addFace()
{
  return Face(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(CELL_ENTITY, 2));
}

/// Add a volume to the manager (without any relationships)
Volume Manager::addVolume()
{
  return Volume(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(CELL_ENTITY, 3));
}

/// Insert a VertexUse at the specified \a uid.
VertexUse Manager::insertVertexUse(const UUID& uid)
{
  return VertexUse(
    shared_from_this(),
    this->setEntityOfTypeAndDimension(uid, USE_ENTITY, 0)->first);
}

/// Create a VertexUse with the specified \a uid and replace \a src's VertexUse.
VertexUse Manager::setVertexUse(const UUID& uid, const Vertex& src, int sense)
{
  VertexUse vertUse = this->insertVertexUse(uid);
  this->findCreateOrReplaceCellUseOfSenseAndOrientation(
    src.entity(), sense, POSITIVE, uid);
  return vertUse;
}

/// Insert a EdgeUse at the specified \a uid.
EdgeUse Manager::insertEdgeUse(const UUID& uid)
{
  return EdgeUse(
    shared_from_this(),
    this->setEntityOfTypeAndDimension(uid, USE_ENTITY, 1)->first);
}

/// Create a EdgeUse with the specified \a uid and replace \a src's EdgeUse.
EdgeUse Manager::setEdgeUse(const UUID& uid, const Edge& src, int sense, Orientation o)
{
  EdgeUse edgeUse = this->insertEdgeUse(uid);
  this->findCreateOrReplaceCellUseOfSenseAndOrientation(
    src.entity(), sense, o, uid);
  return edgeUse;
}

/// Insert a FaceUse at the specified \a uid.
FaceUse Manager::insertFaceUse(const UUID& uid)
{
  return FaceUse(
    shared_from_this(),
    this->setEntityOfTypeAndDimension(uid, USE_ENTITY, 2)->first);
}

/// Create a FaceUse with the specified \a uid and replace \a src's FaceUse.
FaceUse Manager::setFaceUse(const UUID& uid, const Face& src, int sense, Orientation o)
{
  FaceUse faceUse = this->insertFaceUse(uid);
  this->findCreateOrReplaceCellUseOfSenseAndOrientation(
    src.entity(), sense, o, uid);
  return faceUse;
}

/// Insert a VolumeUse at the specified \a uid.
VolumeUse Manager::insertVolumeUse(const UUID& uid)
{
  return VolumeUse(
    shared_from_this(),
    this->setEntityOfTypeAndDimension(uid, USE_ENTITY, 3)->first);
}

/// Create a VolumeUse with the specified \a uid and replace \a src's VolumeUse.
VolumeUse Manager::setVolumeUse(const UUID& uid, const Volume& src)
{
  VolumeUse volUse = this->insertVolumeUse(uid);
  this->findCreateOrReplaceCellUseOfSenseAndOrientation(
    src.entity(), 0, POSITIVE, uid);
  return volUse;
}

/// Add a vertex-use to the manager (without any relationships)
VertexUse Manager::addVertexUse()
{
  return VertexUse(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(USE_ENTITY, 0));
}

/// Find or add a vertex-use to the manager with a relationship back to a vertex.
VertexUse Manager::addVertexUse(const Vertex& src, int sense)
{
  if (src.isValid() && src.manager().get() == this)
    {
    return VertexUse(
      src.manager(),
      this->findCreateOrReplaceCellUseOfSenseAndOrientation(src.entity(), sense, POSITIVE));
    }
  return VertexUse(); // invalid vertex use if source vertex was invalid or from a different manager.
}

/// Add an edge-use to the manager (without any relationships)
EdgeUse Manager::addEdgeUse()
{
  return EdgeUse(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(USE_ENTITY, 1));
}

/// Find or add a edge-use to the manager with a relationship back to a edge.
EdgeUse Manager::addEdgeUse(const Edge& src, int sense, Orientation orient)
{
  if (src.isValid() && src.manager().get() == this)
    {
    return EdgeUse(
      src.manager(),
      this->findCreateOrReplaceCellUseOfSenseAndOrientation(src.entity(), sense, orient));
    }
  return EdgeUse(); // invalid edge use if source edge was invalid or from a different manager.
}

/// Add a face-use to the manager (without any relationships)
FaceUse Manager::addFaceUse()
{
  return FaceUse(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(USE_ENTITY, 2));
}

/// Find or add a face-use to the manager with a relationship back to a face.
FaceUse Manager::addFaceUse(const Face& src, int sense, Orientation orient)
{
  if (src.isValid() && src.manager().get() == this)
    {
    return FaceUse(
      src.manager(),
      src.manager()->findCreateOrReplaceCellUseOfSenseAndOrientation(src.entity(), sense, orient));
    }
  return FaceUse(); // invalid face use if source face was invalid or from a different manager.
}

/// Add a volume-use to the manager (without any relationships)
VolumeUse Manager::addVolumeUse()
{
  return VolumeUse(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(USE_ENTITY, 3));
}

/// Find or add a volume-use to the manager with a relationship back to a volume.
VolumeUse Manager::addVolumeUse(const Volume& src)
{
  if (src.isValid() && src.manager().get() == this)
    {
    return VolumeUse(
      src.manager(),
      src.manager()->findCreateOrReplaceCellUseOfSenseAndOrientation(src.entity(), 0, POSITIVE));
    }
  return VolumeUse(); // invalid volume use if source volume was invalid or from a different manager.
}

/// Insert a Chain at the specified \a uid.
Chain Manager::insertChain(const UUID& uid)
{
  return Chain(
    shared_from_this(),
    this->setEntityOfTypeAndDimension(uid, SHELL_ENTITY | DIMENSION_0 | DIMENSION_1, -1)->first);
}

/// Find or add a chain to the manager with a relationship back to its owning edge-use.
Chain Manager::setChain(const UUID& uid, const EdgeUse& use)
{
  Chain chain = this->insertChain(uid);
  this->findOrAddIncludedShell(use.entity(), uid);
  return chain;
}

/// Find or add a chain to the manager with a relationship back to its owning chain.
Chain Manager::setChain(const UUID& uid, const Chain& parent)
{
  Chain chain = this->insertChain(uid);
  this->findOrAddIncludedShell(parent.entity(), uid);
  return chain;
}


/// Insert a Loop at the specified \a uid.
Loop Manager::insertLoop(const UUID& uid)
{
  return Loop(
    shared_from_this(),
    this->setEntityOfTypeAndDimension(uid, SHELL_ENTITY | DIMENSION_1 | DIMENSION_2, -1)->first);
}

/// Find or add a chain to the manager with a relationship back to its owning face-use.
Loop Manager::setLoop(const UUID& uid, const FaceUse& use)
{
  Loop loop = this->insertLoop(uid);
  this->findOrAddIncludedShell(use.entity(), uid);
  return loop;
}

/// Find or add a chain to the manager with a relationship back to its owning loop.
Loop Manager::setLoop(const UUID& uid, const Loop& parent)
{
  Loop loop = this->insertLoop(uid);
  this->findOrAddIncludedShell(parent.entity(), uid);
  return loop;
}


/// Insert a Shell at the specified \a uid.
Shell Manager::insertShell(const UUID& uid)
{
  return Shell(
    shared_from_this(),
    this->setEntityOfTypeAndDimension(uid, SHELL_ENTITY | DIMENSION_2 | DIMENSION_3, -1)->first);
}

/// Find or add a chain to the manager with a relationship back to its owning volume-use.
Shell Manager::setShell(const UUID& uid, const VolumeUse& use)
{
  Shell shell = this->insertShell(uid);
  this->findOrAddIncludedShell(use.entity(), uid);
  return shell;
}

/// Find or add a chain to the manager with a relationship back to its owning shell.
Shell Manager::setShell(const UUID& uid, const Shell& parent)
{
  Shell shell = this->insertShell(uid);
  this->findOrAddIncludedShell(parent.entity(), uid);
  return shell;
}


/// Add a 0/1-d shell (a vertex chain) to the manager (without any relationships)
Chain Manager::addChain()
{
  return Chain(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(SHELL_ENTITY | DIMENSION_0 | DIMENSION_1, -1));
}

/// Add a 0/1-d shell (a vertex chain) to the manager with a relation to its edge use
Chain Manager::addChain(const EdgeUse& eu)
{
  if (eu.isValid() && eu.manager().get() == this)
    {
    return Chain(
      eu.manager(),
      eu.manager()->createIncludedShell(eu.entity()));
    }
  return Chain();
}

/// Add a 0/1-d shell (a vertex chain) to the manager with a relation to its edge use
Chain Manager::addChain(const Chain& c)
{
  if (c.isValid() && c.manager().get() == this)
    {
    return Chain(
      c.manager(),
      c.manager()->createIncludedShell(c.entity()));
    }
  return Chain();
}

/// Add a 1/2-d shell (an edge loop) to the manager (without any relationships)
Loop Manager::addLoop()
{
  return Loop(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(SHELL_ENTITY | DIMENSION_1 | DIMENSION_2, -1));
}

/// Add a 1/2-d shell (an edge loop) to the manager with a relation to its parent face use
Loop Manager::addLoop(const FaceUse& fu)
{
  if (fu.isValid() && fu.manager().get() == this)
    {
    return Loop(
      fu.manager(),
      fu.manager()->createIncludedShell(fu.entity()));
    }
  return Loop();
}

/// Add a 1/2-d shell (an edge loop) to the manager with a relation to its parent loop
Loop Manager::addLoop(const Loop& lp)
{
  if (lp.isValid() && lp.manager().get() == this)
    {
    return Loop(
      lp.manager(),
      lp.manager()->createIncludedShell(lp.entity()));
    }
  return Loop();
}

/// Add a 2/3-d shell (a face-shell) to the manager (without any relationships)
Shell Manager::addShell()
{
  return Shell(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(SHELL_ENTITY | DIMENSION_2 | DIMENSION_3, -1));
}

/// A convenience method to find or create a volume use for the volume plus a shell.
Shell Manager::addShell(const Volume& v)
{
  VolumeUse vu;
  if (!v.uses<VolumeUses>().empty())
    {
    vu = v.uses<VolumeUses>()[0];
    }
  if (!vu.isValid())
    {
    vu = this->addVolumeUse(v);
    }
  return this->addShell(vu);
}

/// Add a 2/3-d shell (an face shell) to the manager with a relation to its volume
Shell Manager::addShell(const VolumeUse& v)
{
  if (v.isValid() && v.manager().get() == this)
    {
    return Shell(
      v.manager(),
      v.manager()->createIncludedShell(v.entity()));
    }
  return Shell();
}

/**\brief Add an entity group to the manager (without any relationships).
  *
  * Any non-zero bits set in \a extraFlags are OR'd with entityFlags() of the group.
  * This is an easy way to constrain the entities allowed to be members
  * of the group.
  *
  * You may also specify a \a name for the group. If \a name is empty, then no
  * name is assigned.
  */
Group Manager::insertGroup(
  const UUID& uid, int extraFlags, const std::string& groupName)
{
  UUIDWithEntity result =
    this->setEntityOfTypeAndDimension(uid, GROUP_ENTITY | extraFlags, -1);
  if (result == this->m_topology->end())
    return Group();

  if (!groupName.empty())
    this->setStringProperty(uid, "name", groupName);

  return Group(shared_from_this(), uid);
}

/// Add a group, creating a new UUID in the process. \sa insertGroup().
Group Manager::addGroup(int extraFlags, const std::string& groupName)
{
  UUID uid = this->unusedUUID();
  return this->insertGroup(uid, extraFlags, groupName);
}

/**\brief Add a model to the manager.
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
Model Manager::insertModel(
  const UUID& uid,
  int parametricDim, int embeddingDim, const std::string& modelName)
{
  UUIDWithEntity result =
    this->setEntityOfTypeAndDimension(uid, MODEL_ENTITY, parametricDim);
  if (result == this->m_topology->end())
    return Model();

  if (embeddingDim > 0)
    {
    this->setIntegerProperty(uid, "embedding dimension", embeddingDim);
    }

  if (!modelName.empty())
    this->setStringProperty(uid, "name", modelName);
  else
    this->assignDefaultName(uid, this->type(uid));

  return Model(shared_from_this(), uid);
}

/// Add a model, creating a new UUID at the time. \sa insertModel().
Model Manager::addModel(
  int parametricDim, int embeddingDim, const std::string& modelName)
{
  UUID uid = this->unusedUUID();
  return this->insertModel(uid, parametricDim, embeddingDim, modelName);
}

/**\brief Add an instance of some model entity to the manager.
  *
  * An instance is a reference to some other item in the manager.
  * Any entity may be instanced, but generally models are instanced
  * as part of a scene graph.
  */
Instance Manager::addInstance()
{
  UUID uid = this->addEntityOfTypeAndDimension(INSTANCE_ENTITY, -1);
  return Instance(shared_from_this(), uid);
}

/**\brief Add an instance with the given prototype to the manager.
  *
  * The prototype \a object (the parent of the instance)
  * is a reference to some other item in the manager.
  * Any entity may be instanced, but generally models are instanced
  * as part of a scene graph.
  */
Instance Manager::addInstance(const EntityRef& object)
{
  if (object.isValid())
    {
    UUID uid = this->addEntityOfTypeAndDimension(INSTANCE_ENTITY, -1);
    int iidx = this->findEntity(object.entity())->findOrAppendRelation(uid);
    int oidx = this->findEntity(uid)->findOrAppendRelation(object.entity());
    this->arrangeEntity(uid, INSTANCE_OF, Arrangement::InstanceInstanceOfWithIndex(oidx));
    this->arrangeEntity(object.entity(), INSTANCED_BY, Arrangement::InstanceInstanceOfWithIndex(iidx));
    return Instance(shared_from_this(), uid);
    }
  return Instance();
}
//@}

/**\brief Unregister a session session from the model manager.
  *
  */
void Manager::closeSession(const SessionRef& sess)
{
  if (sess.manager().get() == this)
    {
    this->erase(sess);
    this->unregisterSession(sess.session());
    }
}

/**\brief Return an array of all the session sessions this manager owns.
  *
  */
SessionRefs Manager::sessions() const
{
  SessionRefs result;
  UUIDsToSessions::const_iterator it;
  for (it = this->m_sessions->begin(); it != this->m_sessions->end(); ++it)
    result.push_back(
      SessionRef(
        smtk::const_pointer_cast<Manager>(shared_from_this()),
        it->first));
  return result;
}

/**@name Callback methods
  *\brief These methods provide observers with a way to register
  *       and receive notifications of modeling events.
  *
  * Events have 1 of 3 different signatures based on the type of event:
  *
  * + When a single entity is modified independently of changes to others,
  *   a ConditionCallback is invoked.
  * + When a one-to-one relationship between two entities is affected,
  *   a OneToOneCallback is invoked.
  * + When a one-to-many (or a many-to-one) relationship between entities
  *   is affected, a OneToManyCallback is invoked.
  */
//@{
/// Request notification from this manager instance when \a event occurs.
void Manager::observe(ManagerEventType event, ConditionCallback functionHandle, void* callData)
{
  if (event.first == ANY_EVENT)
    {
    int i;
    int iend = static_cast<int>(ANY_EVENT);
    for (i = static_cast<int>(ADD_EVENT); i != iend; ++i)
      {
      event.first = static_cast<ManagerEventChangeType>(i);
      this->observe(event, functionHandle, callData);
      }

    return;
    }

  this->m_conditionTriggers.insert(
    ConditionTrigger(event,
      ConditionObserver(functionHandle, callData)));
}

/// Request notification from this manager instance when \a event occurs.
void Manager::observe(ManagerEventType event, OneToOneCallback functionHandle, void* callData)
{
  if (event.first == ANY_EVENT)
    {
    int i;
    int iend = static_cast<int>(ANY_EVENT);
    for (i = static_cast<int>(ADD_EVENT); i != iend; ++i)
      {
      event.first = static_cast<ManagerEventChangeType>(i);
      this->observe(event, functionHandle, callData);
      }

    return;
    }

  this->m_oneToOneTriggers.insert(
    OneToOneTrigger(event,
      OneToOneObserver(functionHandle, callData)));
}

/// Request notification from this manager instance when \a event occurs.
void Manager::observe(ManagerEventType event, OneToManyCallback functionHandle, void* callData)
{
  if (event.first == ANY_EVENT)
    {
    int i;
    int iend = static_cast<int>(ANY_EVENT);
    for (i = static_cast<int>(ADD_EVENT); i != iend; ++i)
      {
      event.first = static_cast<ManagerEventChangeType>(i);
      this->observe(event, functionHandle, callData);
      }

    return;
    }

  this->m_oneToManyTriggers.insert(
    OneToManyTrigger(event,
      OneToManyObserver(functionHandle, callData)));
}

/// Decline further notification from this manager instance when \a event occurs.
void Manager::unobserve(ManagerEventType event, ConditionCallback functionHandle, void* callData)
{
  if (event.first == ANY_EVENT)
    {
    int i;
    int iend = static_cast<int>(ANY_EVENT);
    for (i = static_cast<int>(ADD_EVENT); i != iend; ++i)
      {
      event.first = static_cast<ManagerEventChangeType>(i);
      this->unobserve(event, functionHandle, callData);
      }

    return;
    }

  this->m_conditionTriggers.erase(
    ConditionTrigger(event,
      ConditionObserver(functionHandle, callData)));
}

/// Decline further notification from this manager instance when \a event occurs.
void Manager::unobserve(ManagerEventType event, OneToOneCallback functionHandle, void* callData)
{
  if (event.first == ANY_EVENT)
    {
    int i;
    int iend = static_cast<int>(ANY_EVENT);
    for (i = static_cast<int>(ADD_EVENT); i != iend; ++i)
      {
      event.first = static_cast<ManagerEventChangeType>(i);
      this->unobserve(event, functionHandle, callData);
      }

    return;
    }

  this->m_oneToOneTriggers.erase(
    OneToOneTrigger(event,
      OneToOneObserver(functionHandle, callData)));
}

/// Decline further notification from this manager instance when \a event occurs.
void Manager::unobserve(ManagerEventType event, OneToManyCallback functionHandle, void* callData)
{
  if (event.first == ANY_EVENT)
    {
    int i;
    int iend = static_cast<int>(ANY_EVENT);
    for (i = static_cast<int>(ADD_EVENT); i != iend; ++i)
      {
      event.first = static_cast<ManagerEventChangeType>(i);
      this->unobserve(event, functionHandle, callData);
      }

    return;
    }

  this->m_oneToManyTriggers.erase(
    OneToManyTrigger(event,
      OneToManyObserver(functionHandle, callData)));
}

/// Called by this Manager instance or EntityRef instances referencing it when \a event occurs.
void Manager::trigger(ManagerEventType event, const smtk::model::EntityRef& src)
{
  std::set<ConditionTrigger>::const_iterator begin =
    this->m_conditionTriggers.lower_bound(
      ConditionTrigger(event,
        ConditionObserver(ConditionCallback(), static_cast<void*>(NULL))));
  std::set<ConditionTrigger>::const_iterator end =
    this->m_conditionTriggers.upper_bound(
      ConditionTrigger(std::make_pair(event.first,static_cast<ManagerEventRelationType>(event.second + 1)),
        ConditionObserver(ConditionCallback(), static_cast<void*>(NULL))));
  for (std::set<ConditionTrigger>::const_iterator it = begin; it != end; ++it)
    (*it->second.first)(it->first, src, it->second.second);
}

/// Called by this Manager instance or EntityRef instances referencing it when \a event occurs.
void Manager::trigger(ManagerEventType event, const smtk::model::EntityRef& src, const smtk::model::EntityRef& related)
{
  std::set<OneToOneTrigger>::const_iterator begin =
    this->m_oneToOneTriggers.lower_bound(
      OneToOneTrigger(event,
        OneToOneObserver(OneToOneCallback(), static_cast<void*>(NULL))));
  std::set<OneToOneTrigger>::const_iterator end =
    this->m_oneToOneTriggers.upper_bound(
      OneToOneTrigger(std::make_pair(event.first,static_cast<ManagerEventRelationType>(event.second + 1)),
        OneToOneObserver(OneToOneCallback(), static_cast<void*>(NULL))));
  for (std::set<OneToOneTrigger>::const_iterator it = begin; it != end; ++it)
    (*it->second.first)(it->first, src, related, it->second.second);
}

/// Called by this Manager instance or EntityRef instances referencing it when \a event occurs.
void Manager::trigger(ManagerEventType event, const smtk::model::EntityRef& src, const smtk::model::EntityRefArray& related)
{
  std::set<OneToManyTrigger>::const_iterator begin =
    this->m_oneToManyTriggers.lower_bound(
      OneToManyTrigger(event,
        OneToManyObserver(OneToManyCallback(), static_cast<void*>(NULL))));
  std::set<OneToManyTrigger>::const_iterator end =
    this->m_oneToManyTriggers.upper_bound(
      OneToManyTrigger(std::make_pair(event.first,static_cast<ManagerEventRelationType>(event.second + 1)),
        OneToManyObserver(OneToManyCallback(), static_cast<void*>(NULL))));
  for (std::set<OneToManyTrigger>::const_iterator it = begin; it != end; ++it)
    (*it->second.first)(it->first, src, related, it->second.second);
}
//@}

  } // namespace model
} //namespace smtk
