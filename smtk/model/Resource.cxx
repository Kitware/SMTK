//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"

#include "smtk/model/AttributeAssignments.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Chain.h"
#include "smtk/model/DefaultSession.h"
#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/EntityIterator.h"
#include "smtk/model/EntityRefArrangementOps.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Model.h"
#include "smtk/model/SessionRef.h"
#include "smtk/model/Shell.h"
#include "smtk/model/ShellEntity.txx"
#include "smtk/model/Vertex.h"
#include "smtk/model/VertexUse.h"
#include "smtk/model/Volume.h"
#include "smtk/model/VolumeUse.h"
#include "smtk/model/queries/SelectionFootprint.h"

#include "smtk/mesh/core/Resource.h"

#include "smtk/common/UUIDGenerator.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"

#include <cfloat>

#include <algorithm>
#include <functional>
#include <iterator>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <vector>

#include <sstream>

//#include <boost/variant.hpp>

using namespace std;
using namespace smtk::common;
using namespace smtk::resource;

namespace smtk
{
namespace model
{

constexpr smtk::resource::Links::RoleType Resource::AssociationRole;
constexpr smtk::resource::Links::RoleType Resource::TessellationRole;

namespace
{
using QueryList = std::tuple<SelectionFootprint>;
}

/**@name Constructors and destructors.
  *\brief Model resource instances should always be created using the static create() method.
  *
  */
//@{
/// Create a default, empty model resource.
Resource::Resource(smtk::resource::ManagerPtr mgr)
  : smtk::resource::DerivedFrom<Resource, smtk::geometry::Resource>(mgr)
  , m_topology(new UUIDsToEntities)
  , m_tessellations(new UUIDsToTessellations)
  , m_analysisMesh(new UUIDsToTessellations)
  , m_attributeAssignments(new UUIDsToAttributeAssignments)
  , m_sessions(new UUIDsToSessions)
  , m_globalCounters(2, 1) // first entry is session counter, second is model counter
{
  // TODO: throw() when topology == nullptr?
  this->queries().registerQueries<QueryList>();
  this->properties().insertPropertyType<smtk::common::UUID>();
}

Resource::Resource(const smtk::common::UUID& uid, smtk::resource::ManagerPtr mgr)
  : smtk::resource::DerivedFrom<Resource, smtk::geometry::Resource>(uid, mgr)
  , m_topology(new UUIDsToEntities)
  , m_tessellations(new UUIDsToTessellations)
  , m_analysisMesh(new UUIDsToTessellations)
  , m_attributeAssignments(new UUIDsToAttributeAssignments)
  , m_sessions(new UUIDsToSessions)
  , m_globalCounters(2, 1) // first entry is session counter, second is model counter
{
  // TODO: throw() when topology == nullptr?
  this->queries().registerQueries<QueryList>();
  this->properties().insertPropertyType<smtk::common::UUID>();
}

/// Create a model resource using the given storage instances.
Resource::Resource(
  shared_ptr<UUIDsToEntities> inTopology,
  shared_ptr<UUIDsToTessellations> tess,
  shared_ptr<UUIDsToTessellations> analysismesh,
  shared_ptr<UUIDsToAttributeAssignments> attribs,
  const smtk::common::UUID& uid,
  smtk::resource::ManagerPtr mgr)
  : smtk::resource::DerivedFrom<Resource, smtk::geometry::Resource>(uid, mgr)
  , m_topology(inTopology)
  , m_tessellations(tess)
  , m_analysisMesh(analysismesh)
  , m_attributeAssignments(attribs)
  , m_sessions(new UUIDsToSessions)
  , m_globalCounters(2, 1) // first entry is session counter, second is model counter
{
  this->queries().registerQueries<QueryList>();
  this->properties().insertPropertyType<smtk::common::UUID>();
}

/// Destroying a model resource requires us to release the default attribute resource..
Resource::~Resource()
{
  if (m_defaultSession)
  {
    // NB: We must pass "false" for the expungeSession argument because
    // the resource may have 0 shared-pointer references at this point
    // and thus cannot construct trigger callback cursors to notify
    // listeners that deletions are occurring.
    this->unregisterSession(m_defaultSession, false);
  }
  m_attributeAssignments->clear();
}
//@}

/**@name Direct member access.
  *\brief These methods provide direct access to the class's storage.
  *
  */
//@{
UUIDsToEntities& Resource::topology()
{
  return *m_topology;
}

const UUIDsToEntities& Resource::topology() const
{
  return *m_topology;
}

UUIDsToTessellations& Resource::tessellations()
{
  return *m_tessellations;
}

const UUIDsToTessellations& Resource::tessellations() const
{
  return *m_tessellations;
}

UUIDsToTessellations& Resource::analysisMesh()
{
  return *m_analysisMesh;
}

const UUIDsToTessellations& Resource::analysisMesh() const
{
  return *m_analysisMesh;
}

bool Resource::setMeshTessellations(const smtk::mesh::ResourcePtr& meshResource)
{
  // We reset the mesh tessellation by first unsetting the existing mesh
  // tessellation (if it exists) and then adding a new link to the input
  // mesh resource. If this becomes a bottleneck, we could add API to
  // smtk::resource::Links to modify the RHS id of the current link,
  // facilitating link modification in place.
  smtk::mesh::ResourcePtr currentMeshTessellations = this->meshTessellations();
  if (currentMeshTessellations != nullptr)
  {
    this->links().removeLinksTo(
      std::static_pointer_cast<smtk::resource::Resource>(currentMeshTessellations),
      TessellationRole);
  }
  return this->links()
           .addLinkTo(
             std::static_pointer_cast<smtk::resource::Resource>(meshResource), TessellationRole)
           .first != smtk::common::UUID::null();
}

smtk::mesh::ResourcePtr Resource::meshTessellations() const
{
  auto tessellationObjects = this->links().linkedTo(TessellationRole);
  return (
    !tessellationObjects.empty()
      ? std::dynamic_pointer_cast<smtk::mesh::Resource>(*tessellationObjects.begin())
      : smtk::mesh::ResourcePtr());
}

void Resource::clear()
{
  m_topology->clear();
  m_tessellations->clear();
  m_analysisMesh->clear();
  {
    smtk::mesh::ResourcePtr currentMeshTessellations = this->meshTessellations();
    if (currentMeshTessellations != nullptr)
    {
      this->links().removeLinksTo(
        std::static_pointer_cast<smtk::resource::Resource>(currentMeshTessellations),
        TessellationRole);
    }
  }
  m_attributeAssignments->clear();
  m_sessions->clear();
  m_attributeResources.clear();
  m_defaultSession = nullptr;
  m_globalCounters.clear();
  // TODO: remove triggers?
}

const UUIDsToAttributeAssignments& Resource::attributeAssignments() const
{
  return *m_attributeAssignments;
}
//@}

/**\brief Remove the entity with the given \a uid.
  *
  * Returns true upon success, false when the entity did not exist.
  *
  * Note that the implementation is aware of when the Resource
  * is actually a Resource and removes storage from the Resource as
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
SessionInfoBits Resource::erase(const UUID& uid, SessionInfoBits flags)
{
  SessionInfoBits actual = flags;
  if (flags & SESSION_ENTITY_RELATIONS)
    actual |= SESSION_ARRANGEMENTS;

  UUIDWithEntityPtr ent;
  bool haveEnt = false;
  if (actual & (SESSION_ENTITY_TYPE | SESSION_ENTITY_RELATIONS))
  {
    ent = m_topology->find(uid);
    if (ent != m_topology->end())
    {
      haveEnt = true;
      if (flags & SESSION_ENTITY_TYPE)
      {
        // Trigger an event before the erasure so the observers
        // have a chance to see what's about to disappear.
        this->trigger(
          std::make_pair(DEL_EVENT, ENTITY_ENTRY), EntityRef(this->shared_from_this(), uid));
      }
    }
    else
    { // without an Entity record, we cannot erase these things:
      actual &= ~(SESSION_ENTITY_TYPE | SESSION_ENTITY_RELATIONS | SESSION_ARRANGEMENTS);
    }
  }

  if (haveEnt && (flags & SESSION_ARRANGEMENTS))
  {
    auto ad(ent->second->arrangementMap());
    for (auto ak = ad.begin(); !ad.empty();)
    {
      ak = ad.begin();
      Arrangements::size_type aidx = ak->second.size();
      for (; aidx > 0; --aidx)
      {
        ent->second->unarrange(ak->first, static_cast<int>(aidx - 1), false);
      }
      ad.erase(ak);
    }
  }

  if (actual & SESSION_TESSELLATION)
    this->tessellations().erase(uid);

  if (actual & SESSION_ATTRIBUTE_ASSOCIATIONS)
    m_attributeAssignments->erase(uid);

  // TODO: If this entity is a model and has parents, we should make
  //       the parent own the child models? Erase the children? Leave
  //       the children as orphans?

  if (haveEnt)
  {
    // Before removing the entity, loop through its relations and
    // make sure none of them retain any references back to \a uid.
    // However, we cannot erase entries in relatedEntity->relations()
    // because relatedEntity's arrangements reference them by integer
    // index. Thus, we call elideEntityReferences rather than removeEntityReferences.
    this->elideEntityReferences(ent);
  }

  typedef resource::Properties::Indexed<std::vector<double>> FloatProperty;
  typedef resource::Properties::Indexed<std::vector<std::string>> StringProperty;
  typedef resource::Properties::Indexed<std::vector<long>> IntProperty;

  // TODO: Notify observers of property removal?
  if (actual & SESSION_USER_DEFINED_PROPERTIES)
  {
    if (actual & SESSION_FLOAT_PROPERTIES)
    {
      this->properties().data().eraseIdForType<FloatProperty>(uid);
    }
    if (actual & SESSION_STRING_PROPERTIES)
    {
      this->properties().data().eraseIdForType<StringProperty>(uid);
    }
    if (actual & SESSION_INTEGER_PROPERTIES)
    {
      this->properties().data().eraseIdForType<IntProperty>(uid);
    }
  }
  else if (actual & SESSION_PROPERTIES)
  {
    SessionRef sref(shared_from_this(), uid);
    if (!sref.isValid())
    {
      Model owningModel(shared_from_this(), this->modelOwningEntity(uid));
      sref = owningModel.session();
    }
    if (sref.session())
      sref.session()->removeGeneratedProperties(EntityRef(shared_from_this(), uid), actual);
  }

  if (haveEnt)
  {
    // TODO: Notify model of entity removal?
    //       Note this can be complicated because removal
    //       of entities in the class destructor prevent us
    //       from obtaining a shared pointer to the resource
    //       to pass to any observers...
    m_topology->erase(uid);
  }

  return actual;
}

/**\brief A convenience method for erasing an entity from storage.
  *
  */
SessionInfoBits Resource::erase(const EntityRef& entityref, SessionInfoBits flags)
{
  return this->Resource::erase(entityref.entity(), flags);
}

/**\brief A convenience method for erasing an entity from storage.
  *
  */
SessionInfoBits Resource::erase(const EntityPtr& entityref, SessionInfoBits flags)
{
  return this->Resource::erase(entityref->id(), flags);
}

/**\brief Erase records related to the entity with no clean or safety checks.
  *
  * Returns bit flags indicating what types of data were erased.
  *
  * **Warning**: If \a flags contains SESSION_PROPERTIES and *not* SESSION_USER_DEFINED_PROPERTIES,
  * then \a eref will be queried for its owning model and the model for its owning session.
  * This can lead to problems if relationships have been erased by previous calls to hardErase.
  */
SessionInfoBits Resource::hardErase(const EntityRef& eref, SessionInfoBits flags)
{
  const smtk::common::UUID& uid(eref.entity());
  if (!uid)
  {
    return SessionInfoBits(0);
  }

  SessionInfoBits actual = flags;
  if (flags & SESSION_ENTITY_RELATIONS)
    actual |= SESSION_ARRANGEMENTS;

  UUIDWithEntityPtr ent;
  if (actual & (SESSION_ENTITY_TYPE | SESSION_ENTITY_RELATIONS | SESSION_ARRANGEMENTS))
  {
    if (!m_topology->erase(uid))
    { // without an Entity record, we cannot erase these things:
      actual &= ~(SESSION_ENTITY_TYPE | SESSION_ENTITY_RELATIONS | SESSION_ARRANGEMENTS);
    }
  }

  if (actual & SESSION_TESSELLATION)
  {
    if (!this->tessellations().erase(uid))
    {
      actual &= ~SESSION_TESSELLATION;
    }
  }

  if (actual & SESSION_ATTRIBUTE_ASSOCIATIONS)
  {
    if (!m_attributeAssignments->erase(uid))
    {
      actual &= ~SESSION_ATTRIBUTE_ASSOCIATIONS;
    }
  }

  if (actual & SESSION_USER_DEFINED_PROPERTIES)
  {
    if (actual & SESSION_FLOAT_PROPERTIES)
    {
      this->properties().data().eraseIdForType<std::vector<double>>(uid);
      if (this->properties().data().containsType<std::vector<double>>())
      {
        actual &= ~SESSION_FLOAT_PROPERTIES;
      }
    }
    if (actual & SESSION_STRING_PROPERTIES)
    {
      this->properties().data().eraseIdForType<std::vector<std::string>>(uid);
      if (this->properties().data().containsType<std::vector<std::string>>())
      {
        actual &= ~SESSION_FLOAT_PROPERTIES;
      }
    }
    if (actual & SESSION_INTEGER_PROPERTIES)
    {
      this->properties().data().eraseIdForType<std::vector<long>>(uid);
      if (this->properties().data().containsType<std::vector<long>>())
      {
        actual &= ~SESSION_FLOAT_PROPERTIES;
      }
    }
  }
  else if (actual & SESSION_PROPERTIES)
  {
    SessionRef sref(shared_from_this(), uid);
    if (!sref.isValid())
    {
      Model owningModel(shared_from_this(), this->modelOwningEntity(uid));
      sref = owningModel.session();
    }
    if (sref.session())
    {
      sref.session()->removeGeneratedProperties(EntityRef(shared_from_this(), uid), actual);
    }
  }
  return actual;
}

/**\brief A convenience method for erasing a model and its children.
  *
  * This removes the model plus all of its free cells, groups, and
  * submodels from storage.
  * This method will have no effect given an invalid model entity.
  */
SessionInfoBits Resource::eraseModel(const Model& model, SessionInfoBits flags)
{
  if (!model.isValid())
    return SESSION_NOTHING;

  CellEntities free = model.cells();
  for (CellEntities::iterator fit = free.begin(); fit != free.end(); ++fit)
  {
    EntityRefs bdys = fit->lowerDimensionalBoundaries(-1);
    for (EntityRefs::iterator bit = bdys.begin(); bit != bdys.end(); ++bit)
    {
      //std::cout << "Erasing " << bit->flagSummary(0) << " " << bit->entity() << "\n";
      this->erase(bit->entity(), flags);
    }
    //std::cout << "Erasing " << fit->flagSummary(0) << " " << fit->entity() << "\n";
    this->erase(fit->entity(), flags);
  }

  Groups grps = model.groups();
  for (Groups::iterator git = grps.begin(); git != grps.end(); ++git)
  {
    EntityRefs members = git->members<EntityRefs>();
    for (EntityRefs::iterator mit = members.begin(); mit != members.end(); ++mit)
    {
      //std::cout << "Erasing " << mit->flagSummary(0) << " " << mit->entity() << "\n";
      this->erase(mit->entity(), flags);
    }
    //std::cout << "Erasing " << git->flagSummary(0) << " " << git->entity() << "\n";
    this->erase(git->entity(), flags);
  }

  //std::cout << "Erasing " << model.flagSummary(0) << " " << model.entity() << "\n";
  this->erase(model.entity(), flags);

  return true;
}

/// Entity construction
//@{
/// Return a currently-unused UUID (guaranteed not to collide if inserted immediately).
UUID Resource::unusedUUID()
{
  UUID actual;
  do
  {
    actual = smtk::common::UUIDGenerator::instance().random();
  } while (m_topology->find(actual) != m_topology->end());
  return actual;
}

/// Insert a new cell of the specified \a dimension, returning an iterator with a new, unique UUID.
Resource::iter_type Resource::insertEntityOfTypeAndDimension(BitFlags entityFlags, int dim)
{
  UUID actual = this->unusedUUID();
  return this->setEntityOfTypeAndDimension(actual, entityFlags, dim);
}

/**\brief Insert the specified cell, returning an iterator.
  *
  * If \a c has a null UUID, one will be generated and set;
  * otherwise the existing UUID will be used when inserting
  * into this class's map.
  */
Resource::iter_type Resource::insertEntity(EntityPtr c)
{
  if (!c->id())
  {
    c->setId(this->unusedUUID());
  }
  return this->setEntity(c);
}

/**\brief Create and map a new cell of the given \a dimension to the given \a uid.
  *
  * Passing a null or non-unique \a uid is an error here and will throw an exception.
  *
  * Some checking and initialization is performed based on \a entityFlags and \a dim,
  * as described below.
  *
  * If the Resource may be cast to a Resource instance and an entity
  * is expected to have a known, fixed number of arrangements of some sort,
  * those are created here so that entityrefs may always rely on their existence
  * even in the absence of the related UUIDs appearing in the entity's relations.
  * For face cells (CELL_2D) entites, two HAS_USE Arrangements are created to
  * reference FaceUse instances.
  */
Resource::iter_type
Resource::setEntityOfTypeAndDimension(const UUID& uid, BitFlags entityFlags, int dim)
{
  UUIDWithEntityPtr it;
  if (uid.isNull())
  {
    std::ostringstream msg;
    msg << "Nil UUID";
    throw msg.str();
  }
  if (
    ((it = m_topology->find(uid)) != m_topology->end()) && (entityFlags & GROUP_ENTITY) != 0 &&
    dim >= 0 && it->second->dimension() != dim)
  {
    std::ostringstream msg;
    msg << "Duplicate UUID '" << uid << "' of different dimension " << it->second->dimension()
        << " != " << dim;
    throw msg.str();
  }
  EntityPtr entrec = Entity::create(entityFlags, dim, shared_from_this());
  entrec->setId(uid);
  std::pair<UUID, EntityPtr> entry(uid, entrec);
  this->prepareForEntity(entry);
  std::pair<Resource::iter_type, bool> result = m_topology->insert(entry);

  if (result.second)
  {
    this->trigger(
      std::make_pair(ADD_EVENT, ENTITY_ENTRY), EntityRef(this->shared_from_this(), uid));
  }

  return result.first;
}

/**\brief Map the specified cell \a c to the given \a uid.
  *
  * Passing a nil or non-unique \a uid is an error here and will throw an exception.
  */
Resource::iter_type Resource::setEntity(EntityPtr c)
{
  UUIDWithEntityPtr it;
  c->reparent(shared_from_this());
  if (c->id().isNull())
  {
    std::ostringstream msg;
    msg << "Nil UUID";
    throw msg.str();
  }
  if ((it = m_topology->find(c->id())) != m_topology->end())
  {
    if (it->second->dimension() != c->dimension())
    {
      std::ostringstream msg;
      msg << "Duplicate UUID '" << c->id() << "' of different dimension " << it->second->dimension()
          << " != " << c->dimension();
      std::cout << msg.str() << std::endl;
      throw msg.str();
    }
    this->removeEntityReferences(it);
    it->second = c;
    this->insertEntityReferences(it);
    return it;
  }
  std::pair<UUID, EntityPtr> entry(c->id(), c);
  this->prepareForEntity(entry);
  it = m_topology->insert(entry).first;
  this->insertEntityReferences(it);
  return it;
}

/// A wrappable version of InsertEntityOfTypeAndDimension
UUID Resource::addEntityOfTypeAndDimension(BitFlags entityFlags, int dim)
{
  return this->insertEntityOfTypeAndDimension(entityFlags, dim)->first;
}

/// A wrappable version of InsertEntity
UUID Resource::addEntity(EntityPtr cell)
{
  return this->insertEntity(cell)->first;
}

/// A wrappable version of SetEntityOfTypeAndDimension
UUID Resource::addEntityOfTypeAndDimensionWithUUID(const UUID& uid, BitFlags entityFlags, int dim)
{
  return this->setEntityOfTypeAndDimension(uid, entityFlags, dim)->first;
}

/// A wrappable version of setEntity. This will override the UUID in \a cell.
UUID Resource::addEntityWithUUID(const UUID& uid, EntityPtr cell)
{
  cell->setId(uid);
  return this->setEntity(cell)->first;
}
//@}

/// Shortcuts for inserting cells with default entity flags.
//@{
Resource::iter_type Resource::insertCellOfDimension(int dim)
{
  return this->insertEntityOfTypeAndDimension(CELL_ENTITY, dim);
}

Resource::iter_type Resource::setCellOfDimension(const UUID& uid, int dim)
{
  return this->setEntityOfTypeAndDimension(uid, CELL_ENTITY, dim);
}

UUID Resource::addCellOfDimension(int dim)
{
  return this->addEntityOfTypeAndDimension(CELL_ENTITY, dim);
}

UUID Resource::addCellOfDimensionWithUUID(const UUID& uid, int dim)
{
  return this->addEntityOfTypeAndDimensionWithUUID(uid, CELL_ENTITY, dim);
}
//@}

/// Queries on entities belonging to the solid.
//@{
/// Return the type of entity that the link represents.
BitFlags Resource::type(const UUID& ofEntity) const
{
  UUIDWithEntityPtr it = m_topology->find(ofEntity);
  return (it == m_topology->end() ? INVALID : it->second->entityFlags());
}

/// Return the dimension of the manifold that the passed entity represents.
int Resource::dimension(const UUID& ofEntity) const
{
  UUIDWithEntityPtr it = m_topology->find(ofEntity);
  return (it == m_topology->end() ? -1 : it->second->dimension());
}

/**\brief Return a name for the given entity ID.
  *
  * This will either return a user-specified name or the "short UUID" name
  * of the entity. It will not assign a name to the entity using the model
  * counters because the method is const.
  */
std::string Resource::name(const UUID& ofEntity) const
{
  if (this->hasStringProperty(ofEntity, "name"))
  {
    smtk::model::StringList const& nprop(this->stringProperty(ofEntity, "name"));
    if (!nprop.empty())
    {
      return nprop[0];
    }
  }
  UUIDWithEntityPtr it = m_topology->find(ofEntity);
  if (it == m_topology->end())
  {
    return "invalid id " + ofEntity.toString();
  }
  return Resource::shortUUIDName(it->first, it->second->entityFlags());
}

/**\brief Return the (Dimension+1 or higher)-entities that are the immediate bordants of the passed entity.
  *
  * \sa HigherDimensionalBoundaries
  */
UUIDs Resource::bordantEntities(const UUID& ofEntity, int ofDimension) const
{
  UUIDs result;
  UUIDWithConstEntityPtr it = m_topology->find(ofEntity);
  if (it == m_topology->end())
  {
    return result;
  }
  if (ofDimension >= 0 && it->second->dimension() >= ofDimension)
  {
    // can't ask for "higher" dimensional boundaries that are lower than the dimension of this cell.
    return result;
  }
  UUIDWithConstEntityPtr other;
  for (UUIDArray::const_iterator ai = it->second->relations().begin();
       ai != it->second->relations().end();
       ++ai)
  {
    other = m_topology->find(*ai);
    if (other == m_topology->end())
    { // TODO: silently skip bad relations or complain?
      continue;
    }
    if (
      (ofDimension >= 0 && other->second->dimension() == ofDimension) ||
      (ofDimension == -2 && other->second->dimension() >= it->second->dimension()))
    { // The dimension is higher, so dumbly push it into the result:
      result.insert(*ai);
    }
    else if (
      (it->second->entityFlags() & CELL_ENTITY) && (other->second->entityFlags() & USE_ENTITY))
    { // ... or it is a use: follow the use upwards.
      ShellEntities bshells = UseEntity(smtk::const_pointer_cast<Resource>(shared_from_this()), *ai)
                                .boundingShellEntities<ShellEntities>();
      for (ShellEntities::iterator shellIt = bshells.begin(); shellIt != bshells.end(); ++shellIt)
      {
        CellEntity cell = shellIt->boundingCell();
        if (cell.dimension() >= ofDimension)
        {
          result.insert(cell.entity());
        }
      }
    }
  }
  return result;
}

/**\brief Return the (Dimension+1 or higher)-entities that are the immediate bordants of any of the passed entities.
  *
  * \sa HigherDimensionalBoundaries
  */
UUIDs Resource::bordantEntities(const UUIDs& ofEntities, int ofDimension) const
{
  UUIDs result;
  std::insert_iterator<UUIDs> inserter(result, result.begin());
  for (UUIDs::const_iterator it = ofEntities.begin(); it != ofEntities.end(); ++it)
  {
    UUIDs brd = this->bordantEntities(*it, ofDimension);
    std::copy(brd.begin(), brd.end(), inserter);
  }
  return result;
}

/**\brief Return the (Dimension-1 or lower)-entities that are the immediate boundary of the passed entity.
  *
  * \sa LowerDimensionalBoundaries
  */
UUIDs Resource::boundaryEntities(const UUID& ofEntity, int ofDimension) const
{
  UUIDs result;
  UUIDWithConstEntityPtr it = m_topology->find(ofEntity);
  if (it == m_topology->end())
  {
    return result;
  }
  if (ofDimension >= 0 && it->second->dimension() <= ofDimension)
  {
    // can't ask for "lower" dimensional boundaries that are higher than the dimension of this cell.
    return result;
  }
  UUIDWithEntityPtr other;
  for (UUIDArray::const_iterator ai = it->second->relations().begin();
       ai != it->second->relations().end();
       ++ai)
  {
    other = m_topology->find(*ai);
    if (other == m_topology->end())
    { // TODO: silently skip bad relations or complain?
      continue;
    }
    if (
      (ofDimension >= 0 && other->second->dimension() == ofDimension) ||
      (ofDimension == -2 && other->second->dimension() <= it->second->dimension() &&
       !other->second->isModel()))
    {
      result.insert(*ai);
    }
    else if (
      (it->second->entityFlags() & CELL_ENTITY) && (other->second->entityFlags() & USE_ENTITY))
    { // ... or it is a use: follow the use downwards.
      ShellEntities shells = UseEntity(smtk::const_pointer_cast<Resource>(shared_from_this()), *ai)
                               .shellEntities<ShellEntities>();
      for (ShellEntities::iterator shellIt = shells.begin(); shellIt != shells.end(); ++shellIt)
      {
        CellEntities cells = shellIt->cellsOfUses<CellEntities>();
        for (CellEntities::iterator cellIt = cells.begin(); cellIt != cells.end(); ++cellIt)
        {
          if (cellIt->dimension() <= ofDimension)
          {
            result.insert(cellIt->entity());
          }
        }
        // Add any inner shells owned by this outer shell. Note the iterator math:
        // since shellIt can be invalidated as we add new entries to the vector,
        // we recompute shellIt after insertion.
        std::size_t itposn = shellIt - shells.begin();
        ShellEntities innerShells = shellIt->containedShellEntities<ShellEntities>();
        shells.insert(shells.end(), innerShells.begin(), innerShells.end());
        shellIt = shells.begin() + itposn;
      }
    }
  }
  return result;
}

/**\brief Return the (Dimension-1 or lower)-entities that are the immediate boundary of any of the passed entities.
  *
  * \sa LowerDimensionalBoundaries
  */
UUIDs Resource::boundaryEntities(const UUIDs& ofEntities, int ofDimension) const
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
  * Unlike Resource::boundaryEntities(), this method will search the boundaries
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
UUIDs Resource::lowerDimensionalBoundaries(const UUID& ofEntity, int lowerDimension)
{
  UUIDs result;
  UUIDWithEntityPtr it = m_topology->find(ofEntity);
  if (it == m_topology->end())
  {
    return result;
  }
  if (it->second->dimension() <= lowerDimension)
  {
    // do nothing
  }
  else
  {
    // FIXME: This only works for the "usual" case where
    //        a cell's relations are dimension (d+1) or
    //        (d-1). We should also collect any out-of-place
    //        relations that match lowerDimension as we go.
    int currentDim = it->second->dimension() - 1;
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
  * Unlike Resource::bordantEntities(), this method will search the bordants
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
UUIDs Resource::higherDimensionalBordants(const UUID& ofEntity, int higherDimension)
{
  UUIDs result;
  UUIDWithEntityPtr it = m_topology->find(ofEntity);
  if (it == m_topology->end())
  {
    return result;
  }
  if (higherDimension >= 0 && it->second->dimension() >= higherDimension)
  {
    // do nothing
  }
  else
  {
    int currentDim = it->second->dimension() + 1;
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
UUIDs Resource::adjacentEntities(const UUID& ofEntity, int ofDimension)
{
  // FIXME: Implement adjacency
  (void)ofEntity;
  (void)ofDimension;
  UUIDs result;
  return result;
}

/// Return all entities of the requested dimension that are present in the solid.
UUIDs Resource::entitiesMatchingFlags(BitFlags mask, bool exactMatch)
{
  UUIDs result;
  for (UUIDWithEntityPtr it = m_topology->begin(); it != m_topology->end(); ++it)
  {
    BitFlags unmasked = it->second->entityFlags();
    BitFlags masked = unmasked & mask;
    // NB: exactMatch still allows some mismatches; specifically, we want to
    //     disregard dimension bits set on models and groups when asking for
    //     exact matches for MODEL_ENTITY and GROUP_ENTITY. Hence the final
    //     condition that (unmasked & ENTITY_MASK) == (mask & ENTITY_MASK)
    //     rather than just unmasked == mask.
    if (
      (masked && (mask == ANY_ENTITY)) || (!exactMatch && masked) ||
      (exactMatch && masked == mask && ((unmasked & ENTITY_MASK) == (mask & ENTITY_MASK))))
    {
      result.insert(it->first);
    }
  }
  return result;
}

/// Return all entities of the requested dimension that are present in the solid.
UUIDs Resource::entitiesOfDimension(int dim)
{
  UUIDs result;
  for (UUIDWithEntityPtr it = m_topology->begin(); it != m_topology->end(); ++it)
  {
    if (it->second->dimension() == dim)
    {
      result.insert(it->first);
    }
  }
  return result;
}
//@}

/**\brief Return the smtk::model::Entity associated with \a uid (or nullptr).
  *
  * Note that even though const, this method may change the records in
  * \a m_topology when \a trySessions is true (as transcription may modify
  * existing entities to insert newly-transcribed relationships)
  * since it may ask a Session instance to fetch a dangling UUID (one
  * marked as existing but un-transcribed) and insert the Entity into
  * Resource. If it is important that entities remain unchanged,
  * call with the second argument (\a trySessions) set to false.
  */
EntityPtr Resource::findEntity(const UUID& uid, bool trySessions) const
{
  UUIDWithEntityPtr it = m_topology->find(uid);
  if (it == m_topology->end())
  {
    // Not in storage... is it in any session's dangling entity list?
    // We use an evil const-cast here because we are working under the fiction
    // that fetching an entity that exists (even if it hasn't been transcribed
    // yet) does not affect storage.
    ResourcePtr self = const_cast<Resource*>(this)->shared_from_this();
    if (trySessions)
    {
      UUIDsToSessions::iterator bit;
      for (bit = self->m_sessions->begin(); bit != self->m_sessions->end(); ++bit)
      {
        if (bit->second->transcribe(EntityRef(self, uid), SESSION_ENTITY_ARRANGED, true))
        {
          it = m_topology->find(uid);
          if (it != m_topology->end())
            return it->second;
        }
      }
    }
    return nullptr;
  }
  return it->second;
}

smtk::resource::ComponentPtr Resource::find(const smtk::common::UUID& uid) const
{
  return std::dynamic_pointer_cast<smtk::resource::Component>(this->findEntity(uid));
}

/// Given a query string, return a functor that determines if a component is
/// accepted by the query.
std::function<bool(const Component&)> Resource::queryOperation(const std::string& queryString) const
{
  return smtk::model::Entity::filterStringToQueryFunctor(queryString);
}

// visit all components in the resource.
void Resource::visit(smtk::resource::Component::Visitor& visitor) const
{
  auto convertedVisitor =
    [&](const std::pair<const smtk::common::UUID, const smtk::model::EntityPtr>& entityPair) {
      const smtk::resource::ComponentPtr resource =
        std::static_pointer_cast<smtk::resource::Component>(entityPair.second);
      visitor(resource);
    };
  std::for_each(m_topology->begin(), m_topology->end(), convertedVisitor);
}

/// Given an entity \a c, ensure that all of its references contain a reference to it.
void Resource::insertEntityReferences(const UUIDWithEntityPtr& c)
{
  UUIDArray::const_iterator bit;
  EntityPtr ref;
  for (bit = c->second->relations().begin(); bit != c->second->relations().end(); ++bit)
  {
    ref = this->findEntity(*bit);
    if (ref)
    {
      ref->appendRelation(c->first);
    }
  }
}

/**\brief Given an entity \a c and the ID of one of its related entities \a r,
  *       overwrite the occurrence of \a r in \a c's array of relations with an
  *       invalid UUID.
  *
  * Use this method to avoid the need to rewrite indices into \a c's
  * array of relations held by \a c and potentially other entities related
  * to \a c.
  */
bool Resource::elideOneEntityReference(const UUIDWithEntityPtr& c, const UUID& r)
{
  UUIDArray::const_iterator bit;
  return c->second->invalidateRelation(r) >= 0;
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
void Resource::elideEntityReferences(const UUIDWithEntityPtr& c)
{
  UUIDArray::const_iterator bit;
  EntityPtr ref;
  for (bit = c->second->relations().begin(); bit != c->second->relations().end(); ++bit)
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
void Resource::removeEntityReferences(const UUIDWithEntityPtr& c)
{
  UUIDArray::const_iterator bit;
  EntityPtr ref;
  for (bit = c->second->relations().begin(); bit != c->second->relations().end(); ++bit)
  {
    ref = this->findEntity(*bit);
    if (ref)
    {
      ref->invalidateRelation(c->first);
    }
  }
}

/**\brief Add entities (specified by their \a uids) to the given group (\a groupId).
  *
  * This will append \a groupId to each entity in \a uids.
  * Note that this does **not** add the proper Arrangement information
  * that Resource::findOrAddEntityToGroup() does, since Resource
  * does not store Arrangement information.
  */
void Resource::addToGroup(const UUID& groupId, const UUIDs& uids)
{
  UUIDWithEntityPtr result = m_topology->find(groupId);
  if (result == m_topology->end())
  {
    return;
  }

  for (UUIDs::const_iterator it = uids.begin(); it != uids.end(); ++it)
  {
    result->second->appendRelation(*it);
  }
  this->insertEntityReferences(result);
}

/** @name Model property accessors.
  *
  */
///@{
void Resource::setFloatProperty(
  const UUID& entity,
  const std::string& propName,
  smtk::model::Float propValue)
{
  typedef resource::Properties::Indexed<std::vector<double>> FloatProperty;
  if (!entity.isNull())
  {
    this->properties().data().get<FloatProperty>()[propName][entity] = { propValue };
  }
}

void Resource::setFloatProperty(
  const UUID& entity,
  const std::string& propName,
  const smtk::model::FloatList& propValue)
{
  typedef resource::Properties::Indexed<std::vector<double>> FloatProperty;
  if (!entity.isNull())
  {
    this->properties().data().get<FloatProperty>()[propName][entity] = propValue;
  }
}

smtk::model::FloatList const& Resource::floatProperty(
  const UUID& entity,
  const std::string& propName) const
{
  typedef resource::Properties::Indexed<std::vector<double>> FloatProperty;
  if (!entity.isNull() && this->hasFloatProperty(entity, propName))
  {
    return this->properties().data().at<FloatProperty>(propName).at(entity);
  }
  static FloatList dummy;
  return dummy;
}

smtk::model::FloatList& Resource::floatProperty(const UUID& entity, const std::string& propName)
{
  typedef resource::Properties::Indexed<std::vector<double>> FloatProperty;
  if (!entity.isNull() && this->hasFloatProperty(entity, propName))
  {
    return this->properties().data().at<FloatProperty>(propName).at(entity);
  }
  static FloatList dummy;
  return dummy;
}

bool Resource::hasFloatProperty(const UUID& entity, const std::string& propName) const
{
  typedef resource::Properties::Indexed<std::vector<double>> FloatProperty;
  if (!entity.isNull() && this->properties().data().contains<FloatProperty>(propName))
  {
    auto it = this->properties().data().at<FloatProperty>(propName).find(entity);
    return (
      it != this->properties().data().at<FloatProperty>(propName).end() && !it->second.empty());
  }
  return false;
}

bool Resource::removeFloatProperty(const UUID& entity, const std::string& propName)
{
  typedef resource::Properties::Indexed<std::vector<double>> FloatProperty;
  if (!entity.isNull())
  {
    auto& map = this->properties().data().at<FloatProperty>(propName);
    auto it = map.find(entity);
    if (it != map.end())
    {
      map.erase(it);
      return true;
    }
  }
  return false;
}

void Resource::setStringProperty(
  const UUID& entity,
  const std::string& propName,
  const smtk::model::String& propValue)
{
  typedef resource::Properties::Indexed<std::vector<std::string>> StringProperty;
  if (!entity.isNull())
  {
    this->properties().data().get<StringProperty>()[propName][entity] = { propValue };
  }
}

void Resource::setStringProperty(
  const UUID& entity,
  const std::string& propName,
  const smtk::model::StringList& propValue)
{
  typedef resource::Properties::Indexed<std::vector<std::string>> StringProperty;
  if (!entity.isNull())
  {
    this->properties().data().get<StringProperty>()[propName][entity] = propValue;
  }
}

smtk::model::StringList const& Resource::stringProperty(
  const UUID& entity,
  const std::string& propName) const
{
  typedef resource::Properties::Indexed<std::vector<std::string>> StringProperty;
  if (!entity.isNull() && this->hasStringProperty(entity, propName))
  {
    return this->properties().data().at<StringProperty>(propName).at(entity);
  }
  static StringList dummy;
  return dummy;
}

smtk::model::StringList& Resource::stringProperty(const UUID& entity, const std::string& propName)
{
  typedef resource::Properties::Indexed<std::vector<std::string>> StringProperty;
  if (!entity.isNull() && this->hasStringProperty(entity, propName))
  {
    return this->properties().data().at<StringProperty>(propName).at(entity);
  }
  static StringList dummy;
  return dummy;
}

bool Resource::hasStringProperty(const UUID& entity, const std::string& propName) const
{
  typedef resource::Properties::Indexed<std::vector<std::string>> StringProperty;
  if (!entity.isNull() && this->properties().data().contains<StringProperty>(propName))
  {
    auto it = this->properties().data().at<StringProperty>(propName).find(entity);
    return (
      it != this->properties().data().at<StringProperty>(propName).end() && !it->second.empty());
  }
  return false;
}

bool Resource::removeStringProperty(const UUID& entity, const std::string& propName)
{
  typedef resource::Properties::Indexed<std::vector<std::string>> StringProperty;
  if (!entity.isNull())
  {
    auto& map = this->properties().data().at<StringProperty>(propName);
    auto it = map.find(entity);
    if (it != map.end())
    {
      map.erase(it);
      return true;
    }
  }
  return false;
}

void Resource::setIntegerProperty(
  const UUID& entity,
  const std::string& propName,
  smtk::model::Integer propValue)
{
  typedef resource::Properties::Indexed<std::vector<long>> IntProperty;
  if (!entity.isNull())
  {
    // this->properties().data().get<IntProperty>()[propName]
    //   .emplace(std::make_pair(entity, propValue));
    this->properties().data().get<IntProperty>()[propName].emplace(
      std::make_pair(smtk::common::UUID(entity), std::vector<long>(1, propValue)));
    // this->properties().data().get<IntProperty>()[propName][entity] = { propValue };
  }
}

void Resource::setIntegerProperty(
  const UUID& entity,
  const std::string& propName,
  const smtk::model::IntegerList& propValue)
{
  typedef resource::Properties::Indexed<std::vector<long>> IntProperty;
  if (!entity.isNull())
  {
    this->properties().data().get<IntProperty>()[propName][entity] = propValue;
  }
}

smtk::model::IntegerList const& Resource::integerProperty(
  const UUID& entity,
  const std::string& propName) const
{
  typedef resource::Properties::Indexed<std::vector<long>> IntProperty;
  if (!entity.isNull() && this->hasIntegerProperty(entity, propName))
  {
    return this->properties().data().at<IntProperty>(propName).at(entity);
  }
  static IntegerList dummy;
  return dummy;
}

smtk::model::IntegerList& Resource::integerProperty(const UUID& entity, const std::string& propName)
{
  typedef resource::Properties::Indexed<std::vector<long>> IntProperty;
  if (!entity.isNull() && this->hasIntegerProperty(entity, propName))
  {
    return this->properties().data().at<IntProperty>(propName).at(entity);
  }
  static IntegerList dummy;
  return dummy;
}

bool Resource::hasIntegerProperty(const UUID& entity, const std::string& propName) const
{
  typedef resource::Properties::Indexed<std::vector<long>> IntProperty;
  if (!entity.isNull() && this->properties().data().contains<IntProperty>(propName))
  {
    auto it = this->properties().data().at<IntProperty>(propName).find(entity);
    return (it != this->properties().data().at<IntProperty>(propName).end() && !it->second.empty());
  }
  return false;
}

bool Resource::removeIntegerProperty(const UUID& entity, const std::string& propName)
{
  typedef resource::Properties::Indexed<std::vector<long>> IntProperty;
  if (!entity.isNull())
  {
    auto& map = this->properties().data().at<IntProperty>(propName);
    auto it = map.find(entity);
    if (it != map.end())
    {
      map.erase(it);
      return true;
    }
  }
  return false;
}
///@}

/// Attempt to find a model owning the given entity.
UUID Resource::modelOwningEntity(const UUID& ent) const
{
  std::set<UUID> visited;
  UUID result = this->modelOwningEntityRecursive(ent, visited);
  return result;
}

/// Attempt to find a session owning the given entity.
UUID Resource::sessionOwningEntity(const UUID& ent) const
{
  std::set<UUID> visited;
  UUID result = this->sessionOwningEntityRecursive(ent, visited);
  return result;
}

/**\brief Assign a string property named "name" to every entity without one.
  *
  * This descends sessions and models owned by sessions rather than
  * blindly iterating over UUIDs; it is thus much faster than calling
  * assignDefaultName() on each entity UUID.
  */
void Resource::assignDefaultNames()
{
  // I. Put every UUID into a bin for processing
  UUIDWithEntityPtr it;
  UUIDs models;  // models that have not had all of their children named
  UUIDs orphans; // entities that may or may not be parent-less
  UUIDs named;   // entities with names
  for (it = m_topology->begin(); it != m_topology->end(); ++it)
  {
    BitFlags etype = it->second->entityFlags();
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

    UUIDWithEntityPtr iit = m_topology->find(*uit);
    this->assignDefaultNamesWithOwner(iit, *uit, oname, orphans, false);
  }
  for (uit = orphans.begin(); uit != orphans.end(); ++uit)
  {
    this->assignDefaultName(*uit);
  }
}

/**\brief Assign a string property named "name" to every entity of a model without one.
  *
  * This descends models rather than blindly iterating over UUIDs;
  * it is thus much faster than calling assignDefaultName() on each entity UUID.
  */
void Resource::assignDefaultNamesToModelChildren(const smtk::common::UUID& modelId)
{
  bool oops = true;
  UUIDWithEntityPtr it = m_topology->find(modelId);
  if (it != m_topology->end())
  {
    Model model(shared_from_this(), modelId);
    if (model.isValid())
    {
      oops = false;
      EntityIterator eit;
      eit.traverse(model, ITERATE_MODELS);
      model.assignDefaultName();
      std::string modelName = model.name();
      UUIDs dummy;
      for (eit.begin(); !eit.isAtEnd(); ++eit)
      {
        if (eit->isSessionRef())
          (*eit).assignDefaultName();
        else
          this->assignDefaultNamesWithOwner(
            m_topology->find(eit->entity()), model.entity(), modelName, dummy, true);
      }
    }
  }
  if (oops)
  {
    smtkWarningMacro(
      this->log(),
      "Tried to assign default names to a non-model entity: "
        << EntityRef(shared_from_this(), modelId).name() << ".");
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
std::string Resource::assignDefaultName(const UUID& uid)
{
  UUIDWithEntityPtr it = m_topology->find(uid);
  if (it != m_topology->end())
  {
    return this->assignDefaultName(it->first, it->second->entityFlags());
  }
  return std::string();
}

/**\brief Assign a string property named "name" to the given entity.
  *
  * This method is like assignDefaultName() but will not replace any
  * pre-existing name.
  */
std::string Resource::assignDefaultNameIfMissing(const UUID& uid)
{
  UUIDWithEntityPtr it = m_topology->find(uid);
  if (it != m_topology->end())
  {
    return this->hasStringProperty(uid, "name")
      ? this->name(uid)
      : this->assignDefaultName(it->first, it->second->entityFlags());
  }
  return std::string();
}

void Resource::assignDefaultNamesWithOwner(
  const UUIDWithEntityPtr& irec,
  const UUID& owner,
  const std::string& ownersName,
  std::set<smtk::common::UUID>& remaining,
  bool nokids)
{
  remaining.erase(irec->first);
  // Assign the item a name if required:
  if (!this->hasStringProperty(irec->first, "name"))
  {
    IntegerList& counts(this->entityCounts(owner, irec->second->entityFlags()));
    if (!this->hasIntegerProperty(irec->first, "pedigree id"))
    {
      int pedigree = Entity::countForType(irec->second->entityFlags(), counts, false);
      this->setIntegerProperty(irec->first, "pedigree id", pedigree);
    }
    std::string defaultName = counts.empty()
      ? Resource::shortUUIDName(irec->first, irec->second->entityFlags())
      : Entity::defaultNameFromCounters(irec->second->entityFlags(), counts);
    this->setStringProperty(irec->first, "name", defaultName);
  }

  if (nokids)
    return;

  // Now descend the owner and assign its children names.
  // Do not ascend... check that relIt dimension decreases or
  // that certain ownership rules are met.
  UUIDArray::const_iterator relIt;
  BitFlags iflg = irec->second->entityFlags();
  BitFlags idim = iflg & ANY_DIMENSION;
  for (relIt = irec->second->relations().begin(); relIt != irec->second->relations().end(); ++relIt)
  {
    UUIDWithEntityPtr child = m_topology->find(*relIt);
    if (child == m_topology->end())
      continue;
    BitFlags cflg = child->second->entityFlags();
    bool yesButNoKids = (cflg & GROUP_ENTITY) && (iflg & MODEL_ENTITY);
    if (
      ((cflg & ANY_DIMENSION) < idim && !(iflg & SHELL_ENTITY)) ||
      ((cflg & SHELL_ENTITY) && (iflg & USE_ENTITY)) ||
      ((cflg & USE_ENTITY) && (iflg & CELL_ENTITY)) || yesButNoKids)
    {
      this->assignDefaultNamesWithOwner(child, owner, ownersName, remaining, yesButNoKids);
    }
  }
}

std::string Resource::assignDefaultName(const UUID& uid, BitFlags entityFlags)
{
  // If this entity is a model, give it a top-level name
  // (even if it is a submodel of some other model -- for brevity).
  if (entityFlags & MODEL_ENTITY)
  {
    std::string tmpName;
    if (!this->hasStringProperty(uid, "name"))
    {
      std::ostringstream defaultName;
      defaultName << "Model ";
      int count = m_globalCounters[1]++;
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
    if (!this->hasStringProperty(uid, "name"))
    {
      std::ostringstream nameStream;
      SessionRef sref(shared_from_this(), uid);
      Session::Ptr sess = sref.session();
      if (sess)
      {
        // if this is a DefaultSession and there is a remote session name, display that;
        // otherwise, show the local session name.
        DefaultSessionPtr defSess = smtk::dynamic_pointer_cast<DefaultSession>(sess);
        nameStream << (defSess && !defSess->remoteName().empty() ? defSess->remoteName()
                                                                 : sess->name())
                   << " models";
      }
      else
      {
        nameStream << Entity::defaultNameFromCounters(entityFlags, m_globalCounters);
      }
      tmpName = nameStream.str();
      this->setStringProperty(uid, "name", tmpName);
    }
    else
    {
      tmpName = this->stringProperty(uid, "name")[0];
    }
    return tmpName;
  }
  // Not a model or session, get its parent's per-type counters:
  UUID owner(this->modelOwningEntity(uid));
  IntegerList& counts(this->entityCounts(owner, entityFlags));
  // Compose a name from the owner and counters:
  if (!this->hasIntegerProperty(uid, "pedigree id"))
  {
    int pedigree = Entity::countForType(entityFlags, counts, false);
    this->setIntegerProperty(uid, "pedigree id", pedigree);
  }
  std::string defaultName = counts.empty() ? Resource::shortUUIDName(uid, entityFlags)
                                           : Entity::defaultNameFromCounters(entityFlags, counts);
  this->setStringProperty(uid, "name", defaultName);
  return defaultName;
}

std::string Resource::shortUUIDName(const UUID& uid, BitFlags entityFlags)
{
  std::string name = Entity::flagSummaryHelper(entityFlags);
  name += "..";
  std::string uidStr = uid.toString();
  name += uidStr.substr(uidStr.size() - 4);
  return name;
}

/// Mark the start of a modeling session by registering the \a session with SMTK backing storage.
SessionRef Resource::registerSession(SessionPtr session)
{
  if (!session)
    return SessionRef();

  UUID sessId = session->sessionId();
  if (sessId.isNull())
    return SessionRef();

  (*m_sessions)[sessId] = session;
  Resource::iter_type brec = this->setEntityOfTypeAndDimension(sessId, SESSION, -1);

  session->setResource(this);
  return SessionRef(shared_from_this(), brec->first);
}

/**\brief Mark the end of a modeling session by removing its \a session.
  *
  * This will remove session-member entities if \a expungeSession is true.
  */
bool Resource::unregisterSession(SessionPtr session, bool expungeSession)
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
    typedef resource::Properties::Indexed<std::vector<double>> FloatProperty;
    typedef resource::Properties::Indexed<std::vector<std::string>> StringProperty;
    typedef resource::Properties::Indexed<std::vector<long>> IntProperty;

    // Remove the session's entity record, properties, and such, but not
    // records, properties, etc. for entities the session owns.
    m_topology->erase(sessId);
    this->properties().data().eraseIdForType<FloatProperty>(sessId);
    this->properties().data().eraseIdForType<StringProperty>(sessId);
    this->properties().data().eraseIdForType<IntProperty>(sessId);
    m_tessellations->erase(sessId);
    m_attributeAssignments->erase(sessId);
  }
  return m_sessions->erase(sessId) != 0;
}

/// Find a session given its session UUID (or nullptr).
SessionPtr Resource::sessionData(const smtk::model::SessionRef& sessId) const
{
  if (sessId.entity().isNull())
    return m_defaultSession;

  UUIDsToSessions::const_iterator it = m_sessions->find(sessId.entity());
  if (it == m_sessions->end())
    return SessionPtr();
  return it->second;
}

/// Return a reference to the \a modelId's counter array associated with the given \a entityFlags.
IntegerList& Resource::entityCounts(const UUID& modelId, BitFlags entityFlags)
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
void Resource::prepareForEntity(std::pair<UUID, EntityPtr>& entry)
{
  if ((entry.second->entityFlags() & CELL_2D) == CELL_2D)
  {
    if (!entry.second->hasArrangementsOfKind(HAS_USE))
    {
      // Create arrangements to hold face-uses:
      entry.second->arrange(
        HAS_USE, Arrangement::CellHasUseWithIndexSenseAndOrientation(-1, 0, NEGATIVE));
      entry.second->arrange(
        HAS_USE, Arrangement::CellHasUseWithIndexSenseAndOrientation(-1, 0, POSITIVE));
    }
  }
  else if (entry.second->entityFlags() & USE_ENTITY)
  {
    if (!entry.second->hasArrangementsOfKind(HAS_SHELL))
    {
      // Create arrangement to hold parent shell:
      entry.second->arrange(HAS_SHELL, Arrangement::UseHasShellWithIndex(-1));
    }
  }
  else if ((entry.second->entityFlags() & MODEL_ENTITY) == MODEL_ENTITY)
  {
    // New models keep counters indicating their local entity counters
    Integer topoCountsData[] = { 0, 0, 0, 0, 0, 0 };
    Integer groupCountsData[] = { 0, 0, 0 };
    Integer otherCountsData[] = { 0 };
    IntegerList topoCounts(
      topoCountsData, topoCountsData + sizeof(topoCountsData) / sizeof(topoCountsData[0]));
    IntegerList groupCounts(
      groupCountsData, groupCountsData + sizeof(groupCountsData) / sizeof(groupCountsData[0]));
    IntegerList otherCounts(
      otherCountsData, otherCountsData + sizeof(otherCountsData) / sizeof(otherCountsData[0]));
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
EntityRefArray Resource::findEntitiesByProperty(const std::string& pname, Integer pval)
{
  return this->findEntitiesByPropertyAs<EntityRefArray>(pname, pval);
}

/// Find entities with a floating-point property named \a pname whose value is the single value \a pval.
EntityRefArray Resource::findEntitiesByProperty(const std::string& pname, Float pval)
{
  return this->findEntitiesByPropertyAs<EntityRefArray>(pname, pval);
}

/// Find entities with a string property named \a pname whose value is the single value \a pval.
EntityRefArray Resource::findEntitiesByProperty(const std::string& pname, const std::string& pval)
{
  return this->findEntitiesByPropertyAs<EntityRefArray>(pname, pval);
}

/// Find entities with an integer property named \a pname whose every value matches the array \a pval.
EntityRefArray Resource::findEntitiesByProperty(const std::string& pname, const IntegerList& pval)
{
  return this->findEntitiesByPropertyAs<EntityRefArray>(pname, pval);
}

/// Find entities with a floating-point property named \a pname whose every value matches the array \a pval.
EntityRefArray Resource::findEntitiesByProperty(const std::string& pname, const FloatList& pval)
{
  return this->findEntitiesByPropertyAs<EntityRefArray>(pname, pval);
}

/// Find entities with a string property named \a pname whose every value matches the array \a pval.
EntityRefArray Resource::findEntitiesByProperty(const std::string& pname, const StringList& pval)
{
  return this->findEntitiesByPropertyAs<EntityRefArray>(pname, pval);
}

/*! \fn template<typename Collection> Collection Resource::findEntitiesByPropertyAs(const std::string& pname, Integer pval)
 * \brief Return entities with a property named \a pname whose values match the given \a pval.
 */

/*! \fn template<typename Collection> Collection Resource::findEntitiesByPropertyAs(const std::string& pname, const IntegerList& pval)
 * \brief Return entities with a property named \a pname whose values match the given \a pval.
 */

/*! \fn template<typename Collection> Collection Resource::findEntitiesByPropertyAs(const std::string& pname, Float pval)
 * \brief Return entities with a property named \a pname whose values match the given \a pval.
 */

/*! \fn template<typename Collection> Collection Resource::findEntitiesByPropertyAs(const std::string& pname, const std::vector< double >& pval)
 * \brief Return entities with a property named \a pname whose values match the given \a pval.
 */

/*! \fn template<typename Collection> Collection Resource::findEntitiesByPropertyAs(const std::string& pname, const std::string& pval)
 * \brief Return entities with a property named \a pname whose values match the given \a pval.
 */

/*! \fn template<typename Collection> Collection Resource::findEntitiesByPropertyAs(const std::string& pname, const std::vector< std::string >& pval)
 * \brief Return entities with a property named \a pname whose values match the given \a pval.
 */
//@}

/**\brief Find entities whose type matches the given \a flags.
  *
  * This version can be wrapped and used in Python.
  * It is not named entitiesMatchingFlags (to mirror the
  * templated entitiesMatchingFlagsAs<T>) because our base
  * class, Resource, provides another method of the same
  * name that returns UUIDs rather than EntityRefArray.
  */
EntityRefArray Resource::findEntitiesOfType(BitFlags flags, bool exactMatch)
{
  return this->entitiesMatchingFlagsAs<EntityRefArray>(flags, exactMatch);
}

/**\brief Set the tessellation information and for a given \a cellId.
  *
  * If \a analysis is non-zero (zero is the default), then the
  * Tessellation is treated as an analysis mesh, not a display
  * tessellation.
  *
  * Note that calling this method automatically sets or increments
  * the integer-valued "_tessgen" property on \a cellId.
  * This property enables fast display updates when only a few
  * entity tessellations have changed.
  * If \a generation is a non-nullptr pointer (nullptr is the default),
  * then the new generation number of the Tessellation is stored at
  * the address provided.
  */
Resource::tess_iter_type Resource::setTessellation(
  const UUID& cellId,
  const Tessellation& geom,
  int analysis,
  int* generation)
{
  if (cellId.isNull())
    throw std::string("Nil cell ID");

  smtk::shared_ptr<UUIDsToTessellations> storage;
  const char* genProp;
  if (!analysis)
  { // store as display tessellation
    storage = m_tessellations;
    genProp = SMTK_TESS_GEN_PROP;
  }
  else
  { // store as analysis mesh
    storage = m_analysisMesh;
    genProp = SMTK_MESH_GEN_PROP;
  }

  tess_iter_type result = storage->find(cellId);
  if (result == storage->end())
  {
    std::pair<UUID, Tessellation> blank;
    blank.first = cellId;
    result = storage->insert(blank).first;
  }
  result->second = geom;

  // Now set or increment the generation number.
  IntegerList& gen(this->integerProperty(cellId, genProp));
  if (gen.empty())
    gen.push_back(0);
  else
    ++gen[0];
  if (generation)
    *generation = gen[0];

  return result;
}

/**\brief Set the tessellation information and bounding box for a given \a cellId.
  *
  * If \a analysis is non-zero (zero is the default), then the
  * Tessellation is treated as an analysis mesh, not a display
  * tessellation.
  *
  * Note that calling this method automatically sets or increments
  * the integer-valued "_tessgen" property on \a cellId.
  * This property enables fast display updates when only a few
  * entity tessellations have changed.
  * If \a generation is a non-nullptr pointer (nullptr is the default),
  * then the new generation number of the Tessellation is stored at
  * the address provided.
  */
Resource::tess_iter_type Resource::setTessellationAndBoundingBox(
  const UUID& cellId,
  const Tessellation& geom,
  int analysis,
  int* generation)
{
  if (cellId.isNull())
    throw std::string("Nil cell ID");

  smtk::shared_ptr<UUIDsToTessellations> storage;
  const char* genProp;
  if (!analysis)
  { // store as display tessellation
    storage = m_tessellations;
    genProp = SMTK_TESS_GEN_PROP;
  }
  else
  { // store as analysis mesh
    storage = m_analysisMesh;
    genProp = SMTK_MESH_GEN_PROP;
  }

  tess_iter_type result = storage->find(cellId);
  if (result == storage->end())
  {
    std::pair<UUID, Tessellation> blank;
    blank.first = cellId;
    result = storage->insert(blank).first;
  }
  result->second = geom;

  // Now set or increment the generation number.
  IntegerList& gen(this->integerProperty(cellId, genProp));
  if (gen.empty())
    gen.push_back(0);
  else
    ++gen[0];
  if (generation)
    *generation = gen[0];

  // Set/upate the bBox
  this->setBoundingBox(cellId, geom.coords());
  return result;
}

/**\brief set the bounding box of a model entity given \a entityId and \a coords.
  *
  * if provided bBox, we would just use the coords as bBox
  *
  * Returns true when a real bBox is set and false otherwise.
  */
bool Resource::setBoundingBox(
  const UUID& cellId,
  const std::vector<double>& coords,
  int providedbBox)
{
  smtk::model::FloatList bBox;
  if (providedbBox)
  {
    this->setFloatProperty(cellId, SMTK_BOUNDING_BOX_PROP, coords);
    return true;
  }
  else // calculate boundingBox
  {
    if (coords.empty())
    {
      return false;
    } // nothing to set
    // initialize the bBox
    for (int i = 0; i < 6; i++)
    {
      double tmp = (i % 2 == 1) ? -DBL_MAX : DBL_MAX;
      bBox.push_back(tmp);
    }
    std::vector<double>::size_type pointSize = coords.size() / 3;
    for (size_t i = 0; i < pointSize; ++i)
    {
      bBox[0] = std::min(bBox[0], coords[3 * i]);
      bBox[1] = std::max(bBox[1], coords[3 * i]);
      bBox[2] = std::min(bBox[2], coords[3 * i + 1]);
      bBox[3] = std::max(bBox[3], coords[3 * i + 1]);
      bBox[4] = std::min(bBox[4], coords[3 * i + 2]);
      bBox[5] = std::max(bBox[5], coords[3 * i + 2]);
    }
    this->setFloatProperty(cellId, SMTK_BOUNDING_BOX_PROP, bBox);
    return true;
  }
}

/**\brief Remove the tessellation of the given \a entityId.
  *
  * If the second argument is true, also remove the integer "generation number"
  * property from the entity.
  *
  * Returns true when a tessellation was actually removed and false otherwise.
  */
bool Resource::removeTessellation(const smtk::common::UUID& entityId, bool removeGen)
{
  UUIDWithTessellation tref = m_tessellations->find(entityId);
  bool canRemove = (tref != m_tessellations->end());
  if (canRemove)
  {
    m_tessellations->erase(tref);

    if (removeGen)
    {
      this->removeIntegerProperty(entityId, SMTK_TESS_GEN_PROP);
    }
  }
  return canRemove;
}

/**\brief Add or replace information about the arrangement of an entity.
  *
  * When \a index is -1, the arrangement is considered new and added to the end of
  * the vector of arrangements of the given \a kind.
  * Otherwise, it should be positive and refer to a pre-existing arrangement to be replaced.
  * The actual \a index location used is returned.
  */
int Resource::arrangeEntity(
  const UUID& entityId,
  ArrangementKind kind,
  const Arrangement& arr,
  int index)
{
  UUIDsToEntities::iterator eit = m_topology->find(entityId);
  if (eit == m_topology->end())
  {
    return -1;
  }
  return eit->second->arrange(kind, arr, index);
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
int Resource::unarrangeEntity(const UUID& entityId, ArrangementKind k, int index, bool removeIfLast)
{
  auto eit = m_topology->find(entityId);
  if (eit == m_topology->end())
  {
    return 0;
  }
  int result = eit->second->unarrange(k, index);

  // If we removed the last arrangement relating this entity to others,
  // and if the caller has requested it: remove the entity itself.
  if (removeIfLast && eit->second->arrangementMap().empty())
  {
    m_topology->erase(eit);
    ++result;
  }

  return result;
}

/**\brief Erase all arrangements for the given \a entityId.
  *
  * \warning
  * Unlike unarrangeEntity(), this method does not alter arrangements
  * for any other entity and thus can leave previously-bidirectional
  * arrangements as unidirectional.
  *
  * Returns true when \a entity had a non-empty dictionary of
  * arrangements and false otherwise.
  *
  * Note that this does not erase the entry in the map from UUIDs
  * to arrangements, but rather clears the arrangement dictionary
  * for the given UUID.
  */
bool Resource::clearArrangements(const smtk::common::UUID& entityId)
{
  auto eit = m_topology->find(entityId);
  if (eit == m_topology->end())
  {
    return false;
  }
  return eit->second->clearArrangements();
}

/**\brief Returns true when the given \a entity has any arrangements of the given \a kind (otherwise false).
  *
  * Use this to avoid accidentally inserting a new array of arrangements with arrangementsOfKindForEntity().
  * Since this actually requires a lookup, you may pass in a pointer \a arr to an array of arrangements;
  * if true is returned, the pointer will be aimed at the existing array. Otherwise, \a arr will be unchanged.
  */
Arrangements* Resource::hasArrangementsOfKindForEntity(const UUID& entity, ArrangementKind kind)
{
  auto eit = m_topology->find(entity);
  if (eit == m_topology->end())
  {
    return nullptr;
  }
  return eit->second->hasArrangementsOfKind(kind);
}

/**\brief This is a const version of hasArrangementsOfKindForEntity().
  */
const Arrangements* Resource::hasArrangementsOfKindForEntity(
  const UUID& entity,
  ArrangementKind kind) const
{
  auto eit = m_topology->find(entity);
  if (eit == m_topology->end())
  {
    return nullptr;
  }
  return eit->second->hasArrangementsOfKind(kind);
}

/**\brief Return an array of arrangements of the given \a kind for the given \a entity.
  *
  * NOTE: This method will create an empty array and attach it to the entity
  * if none exists, thus increasing storage costs. Unless you intend to append
  * new relationships, you should not use this method without first calling
  * hasArrangementsOfKindForEntity() to determine whether the array already exists.
  */
Arrangements& Resource::arrangementsOfKindForEntity(const UUID& entity, ArrangementKind kind)
{
  auto eit = m_topology->find(entity);
  return eit->second->arrangementsOfKind(kind);
}

/**\brief Retrieve arrangement information for a cell.
  *
  * This version does not allow the arrangement to be altered.
  */
const Arrangement* Resource::findArrangement(const UUID& entityId, ArrangementKind kind, int index)
  const
{
  if (!entityId || index < 0)
  {
    return nullptr;
  }

  auto eit = m_topology->find(entityId);
  if (eit == m_topology->end())
  {
    return nullptr;
  }

  return eit->second->findArrangement(kind, index);
}

/**\brief Retrieve arrangement information for an entity.
  *
  * This version allows the arrangement to be altered.
  */
Arrangement* Resource::findArrangement(const UUID& entityId, ArrangementKind kind, int index)
{
  if (!entityId || index < 0)
  {
    return nullptr;
  }

  auto eit = m_topology->find(entityId);
  if (eit == m_topology->end())
  {
    return nullptr;
  }

  return eit->second->findArrangement(kind, index);
}

/**\brief Find an arrangement of type \a kind that relates \a entityId to \a involvedEntity.
  *
  * This method returns the index upon success and a negative number upon failure.
  */
int Resource::findArrangementInvolvingEntity(
  const UUID& entityId,
  ArrangementKind kind,
  const UUID& involvedEntity) const
{
  const EntityPtr src = this->findEntity(entityId);
  if (!src)
    return -1;

  return src->findArrangementInvolvingEntity(kind, involvedEntity);
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
bool Resource::findDualArrangements(
  const UUID& entityId,
  ArrangementKind kind,
  int index,
  ArrangementReferences& duals) const
{
  if (index < 0)
    return false;

  EntityPtr src = this->findEntity(entityId);
  if (!src)
    return false;

  return src->findDualArrangements(kind, index, duals);
}

/**\brief A method to add bidirectional arrangements between a parent and child.
  *
  */
bool Resource::addDualArrangement(
  const smtk::common::UUID& parent,
  const smtk::common::UUID& child,
  ArrangementKind kind,
  int sense,
  Orientation orientation)
{
  EntityPtr prec;
  EntityPtr crec;
  prec = this->findEntity(parent, false);
  if (!prec)
    return false;
  EntityTypeBits parentType = static_cast<EntityTypeBits>(prec->entityFlags() & ENTITY_MASK);
  int childIndex = prec->findOrAppendRelation(child);

  crec = this->findEntity(child, false);
  if (!crec)
    return false;
  EntityTypeBits childType = static_cast<EntityTypeBits>(crec->entityFlags() & ENTITY_MASK);
  int parentIndex = crec->findOrAppendRelation(parent);

  ArrangementKind dualKind = Dual(parentType, kind);
  if (dualKind == KINDS_OF_ARRANGEMENTS)
    return false;

  prec->arrange(kind, Arrangement::Construct(parentType, kind, childIndex, sense, orientation));
  crec->arrange(
    dualKind, Arrangement::Construct(childType, dualKind, parentIndex, sense, orientation));
  return true;
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
  *
  * There should be no duplicate senses for any given cell with the same orientation
  * except in the case of vertex uses.
  * Vertex uses have no orientation and each sense of a vertex corresponds to
  * a unique connected point-set locus in the neighborhood of the domain with
  * the vertex removed.
  * So, a torus pinched to a conical point at one location on its boundary
  * might have a periodic circular edge terminated by the same vertex at each end.
  * However, the sense of the vertex uses for each endpoint would be different
  * since subtracting the vertex from the bi-conic neighborhood yields distinct
  * connected components. (The components are distinct inside small neighborhoods
  * of the vertex even though the components are connected by an edge; this
  * convention should be followed so that it is possible to compute deflection vectors
  * that will remove the degeneracy of the vertex.)
  */
int Resource::findCellHasUseWithSense(const UUID& cellId, const UUID& use, int sense) const
{
  EntityPtr erec = this->findEntity(cellId);
  const Arrangements* arrs = this->hasArrangementsOfKindForEntity(cellId, HAS_USE);
  if (arrs && erec)
  {
    int i = 0;
    for (Arrangements::const_iterator it = arrs->begin(); it != arrs->end(); ++it, ++i)
    {
      int itIdx, itSense;
      Orientation itOrient;
      if (
        it->IndexSenseAndOrientationFromCellHasUse(itIdx, itSense, itOrient) && itIdx >= 0 &&
        erec->relations()[itIdx] == use && itSense == sense)
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
std::set<int> Resource::findCellHasUsesWithOrientation(const UUID& cellId, Orientation orient) const
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
        it->IndexSenseAndOrientationFromCellHasUse(itIdx, itSense, itOrient) && itOrient == orient)
      {
        result.insert(i);
      }
    }
  }
  return result;
}

/**\brief Return the UUID of a use record for the
  * given \a cell and \a sense, or nullptr if it does not exist.
  */
UUID Resource::cellHasUseOfSenseAndOrientation(const UUID& cell, int sense, Orientation orient)
  const
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
  * creating one if it does not exist or replacing it if \a replacement is non-nullptr.
  *
  */
UUID Resource::findCreateOrReplaceCellUseOfSenseAndOrientation(
  const UUID& cell,
  int sense,
  Orientation orient,
  const UUID& replacement)
{
  EntityPtr entity = this->findEntity(cell);
  if (!entity)
  {
    return UUID::null();
  }
  smtk::model::Arrangements& arr(this->arrangementsOfKindForEntity(cell, HAS_USE));

  // See if any of this cell's uses match our sense...
  int arrIdx = -1;
  smtk::model::Arrangements::const_iterator ait;
  int relIdx = -1;
  int arrCtr = 0;
  for (ait = arr.begin(); ait != arr.end(); ++ait, ++arrCtr)
  {
    int itIdx;
    int itSense;
    Orientation itOrient;
    ait->IndexSenseAndOrientationFromCellHasUse(itIdx, itSense, itOrient);
    if (itSense == sense && itOrient == orient)
    {
      relIdx = itIdx;
      if (itIdx >= 0)
      { // Found a valid use. If we have a replacement, use it.
        if (!replacement.isNull())
        {
          this->unarrangeEntity(cell, HAS_USE, arrCtr, false);
          arrIdx = arrCtr;
          break;
        }
        else if (!entity->relations()[itIdx] || !this->findEntity(entity->relations()[itIdx]))
        { // The arrangement is valid but it references a nullptr entity.
          arrIdx = arrCtr;
          break;
        }
        // TODO... emit signals for unarrangement/rearrangement
        return entity->relations()[itIdx];
      }
      else
      { // We found an existing but invalid use... replace it below.
        arrIdx = arrCtr;
        break;
      }
    }
  }

  // ...nope, we need to create a new use with
  // the specified sense relative to the cell.
  // Note that there may still be an entry in arr
  // which we should overwrite (with itIdx == -1).
  UUID use;
  if (replacement.isNull())
  {
    use =
      this
        ->insertEntityOfTypeAndDimension(USE_ENTITY | entity->dimensionBits(), entity->dimension())
        ->first;
    // We must re-fetch entity since inserting the use
    // may have invalidated our reference to it.
    entity = this->findEntity(cell);
    if (relIdx >= 0)
      entity->relations()[relIdx] = use;
    else
      relIdx = entity->findOrAppendRelation(use);
  }
  else
  {
    use = replacement;
    if (relIdx >= 0)
      entity->relations()[relIdx] = use;
    else
      relIdx = entity->findOrAppendRelation(use);
  }

  if (arrIdx >= 0)
  { // We found an existing use and need to replace it.
    this->arrangeEntity(
      cell,
      HAS_USE,
      Arrangement::CellHasUseWithIndexSenseAndOrientation(relIdx, sense, orient),
      arrIdx);
    // Does the use already have a reference back to the cell?
    this->arrangeEntity(
      use,
      HAS_CELL,
      Arrangement::UseHasCellWithIndexAndSense(
        this->findEntity(use)->findOrAppendRelation(cell), sense));
  }
  else
  {
    // We did not find an arrangement of the specified
    // sense and orientation... append it, adding
    // the use to the cell and the cell to the use:
    this->arrangeEntity(
      cell,
      HAS_USE,
      Arrangement::CellHasUseWithIndexSenseAndOrientation(
        entity->appendRelation(use), sense, orient),
      arrIdx);
    EntityPtr useEnt = this->findEntity(use);
    this->arrangeEntity(
      use, HAS_CELL, Arrangement::UseHasCellWithIndexAndSense(useEnt->appendRelation(cell), sense));
  }

  return use;
}

/**\brief Return the UUIDs of all shells included by the given cell-use or shell.
  *
  * Cell-uses of dimension d may include shells that span dimensions d and (d-1).
  * Shells may include other shells of the same dimensionality.
  * These relationships define a hierarchy that enumerate the oriented boundary of
  * the top-level cell-use.
  */
UUIDs Resource::useOrShellIncludesShells(const UUID& cellUseOrShell) const
{
  UUIDs shells;
  EntityPtr ent = this->findEntity(cellUseOrShell);
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
UUID Resource::createIncludedShell(const UUID& useOrShell)
{
  EntityPtr entity = this->findEntity(useOrShell);
  if (!entity)
  {
    return UUID::null();
  }
  int shellDim = (entity->entityFlags() & USE_ENTITY || entity->entityFlags() & CELL_ENTITY)
    ?
    // k-shells span dimensions (d, d-1), where d = dimension of the cell/cell-use:
    entity->dimensionBits() | (entity->dimensionBits() >> 1)
    :
    // included k-shell must have same dimension as parent:
    entity->dimensionBits();
  int indexOfNewShell = static_cast<int>(entity->relations().size());
  UUIDWithEntityPtr shell = this->insertEntityOfTypeAndDimension(SHELL_ENTITY | shellDim, -1);
  this->arrangeEntity(
    useOrShell, INCLUDES, Arrangement::UseOrShellIncludesShellWithIndex(indexOfNewShell));
  // We must re-find the entity record since insertEntityOfTypeAndDimension:
  this->findEntity(useOrShell)->appendRelation(shell->first);
  this->arrangeEntity(
    shell->first,
    EMBEDDED_IN,
    Arrangement::ShellEmbeddedInUseOrShellWithIndex(
      static_cast<int>(shell->second->relations().size())));
  shell->second->appendRelation(useOrShell);

  return shell->first;
}

/** Add a shell to \a parentUseOrShell as an inclusion unless it already exists.
  *
  * Returns true when adding the shell was necessary.
  * Returns false if either entity does not exist or the shell was already owned by the parent.
  */
bool Resource::findOrAddIncludedShell(const UUID& parentUseOrShell, const UUID& shellToInclude)
{
  EntityPtr parEnt = this->findEntity(parentUseOrShell);
  EntityPtr shlEnt = this->findEntity(shellToInclude);
  if (!parEnt || !shlEnt)
  {
    return false;
  }

  int indexOfShell =
    this->findArrangementInvolvingEntity(parentUseOrShell, INCLUDES, shellToInclude);
  if (indexOfShell >= 0)
    return false;

  // Didn't find it. Add both forward and inverse relations.
  this->arrangeEntity(
    parentUseOrShell,
    INCLUDES,
    Arrangement::UseOrShellIncludesShellWithIndex(parEnt->appendRelation(shellToInclude)));
  this->arrangeEntity(
    shellToInclude,
    EMBEDDED_IN,
    Arrangement::ShellEmbeddedInUseOrShellWithIndex(shlEnt->appendRelation(parentUseOrShell)));

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
bool Resource::findOrAddUseToShell(const UUID& shell, const UUID& use)
{
  EntityPtr shellEnt;
  EntityPtr useEnt;
  // Check that the shell and use are valid and that the use has the proper dimension for the shell.
  if ((shellEnt = this->findEntity(shell)) && (useEnt = this->findEntity(use)))
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
        this->arrangeEntity(
          shell, HAS_USE, Arrangement::ShellHasUseWithIndexRange(srsize, srsize + 1));
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
          *this->findArrangement(use, HAS_SHELL, 0) = Arrangement::UseHasShellWithIndex(shellIdx);
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
          use, HAS_SHELL, Arrangement::UseHasShellWithIndex(useEnt->findOrAppendRelation(shell)));
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
bool Resource::findOrAddInclusionToCellOrModel(const UUID& cell, const UUID& inclusion)
{
  EntityPtr cellEnt;
  EntityPtr incEnt;
  // Check that the cell and inclusion are valid and that the inclusion has the proper dimension for the cell.
  if ((cellEnt = this->findEntity(cell)) && (incEnt = this->findEntity(inclusion)))
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
      this->arrangeEntity(
        inclusion, EMBEDDED_IN, Arrangement::CellEmbeddedInEntityWithIndex(cellIdx));
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
bool Resource::findOrAddEntityToGroup(const UUID& grp, const UUID& ent)
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
  return count > 0;
}

smtk::resource::ResourceSet Resource::associations() const
{
  auto associatedObjects = this->links().linkedTo(AssociationRole);
  smtk::resource::ResourceSet resources;
  for (const auto& object : associatedObjects)
  {
    auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(object);
    if (resource != nullptr)
    {
      resources.insert(resource);
    }
  }
  return resources;
}

bool Resource::associate(const smtk::resource::ResourcePtr& resource)
{
  // Resource links allow for multiple links between the same objects. Since
  // associations are unique, we must first check if an association between this
  // resourse and the resource parameter exists.
  return this->links().isLinkedTo(resource, AssociationRole)
    ? true
    : this->links().addLinkTo(resource, AssociationRole).first != smtk::common::UUID::null();
}

bool Resource::disassociate(const smtk::resource::ResourcePtr& resource)
{
  // Resource links allow for multiple links between the same objects. Since
  // associations are unique, we can erase all links from this resource to the
  // input resource that have an association role.
  return this->links().removeLinksTo(resource, AssociationRole);
}

/**@name Attribute association
  *\brief Associate and disassociate attribute values to entities.
  */
//@{
/**\brief Report whether an entity has been assigned an attribute.
  *
  */
bool Resource::hasAttribute(const UUID& attribId, const UUID& toEntity)
{
  UUIDWithAttributeAssignments it = m_attributeAssignments->find(toEntity);
  if (it == m_attributeAssignments->end())
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
bool Resource::associateAttribute(
  smtk::attribute::ResourcePtr attResource,
  const UUID& attribId,
  const UUID& toEntity)
{
  bool allowed = true;
  if (attResource)
  {
    attribute::AttributePtr att = attResource->findAttribute(attribId);
    if (!att || !att->associateEntity(toEntity))
    {
      allowed = false;
    }
    m_attributeResources.insert(attResource);
  }
  if (allowed)
    (*m_attributeAssignments)[toEntity].associateAttribute(attribId);
  return allowed;
}

/**\brief Unassign an attribute from an entity.
  *
  */
bool Resource::disassociateAttribute(
  smtk::attribute::ResourcePtr attResource,
  const UUID& attribId,
  const UUID& fromEntity,
  bool reverse)
{
  bool didRemove = false;
  UUIDWithAttributeAssignments ref = m_attributeAssignments->find(fromEntity);
  if (ref == m_attributeAssignments->end())
  {
    return didRemove;
  }
  if ((didRemove = ref->second.disassociateAttribute(attribId)))
  {
    // If the AttributeAssignments instance is now empty, remove it.
    if (ref->second.attributeIds().empty())
    {
      m_attributeAssignments->erase(ref);
    }
    // Notify the Attribute of the removal
    if (reverse && attResource)
    {
      smtk::attribute::AttributePtr attrib = attResource->findAttribute(attribId);
      // FIXME: Should we check that the resource's refResource
      //        is this Resource instance?
      if (attrib)
      {
        attrib->disassociateEntity(fromEntity, false);
      }
    }
  }
  return didRemove;
}

/**\brief Insert the attributes associated with \a modelEntity into the \a associations set.
  *
  * Returns true when modelEntity had any attributes (whether they were already
  * present in \a associations or not) in any attribute resource and false otherwise.
  *
  * Note that attribute UUIDs may not be stored in any of the attribute resources
  * (e.g., when a model was loaded from a save file but the attributes were not),
  * in which case this method will return false even though attribute UUIDs were
  * stored on the model entity.
  * This way, a return value of true ensures that the set contains at least one entry.
  */
bool Resource::insertEntityAssociations(
  const EntityRef& modelEntity,
  std::set<smtk::attribute::AttributePtr>& associations)
{
  bool didFind = false;
  auto eait = m_attributeAssignments->find(modelEntity.entity());
  if (eait == m_attributeAssignments->end() || eait->second.attributeIds().empty())
  {
    return didFind;
  }
  for (const auto& attribId : eait->second.attributeIds())
  {
    for (const auto& attribResource : m_attributeResources)
    {
      smtk::attribute::Resource::Ptr aresource = attribResource.lock();
      smtk::attribute::AttributePtr att;
      if (aresource && (att = aresource->findAttribute(attribId)))
      {
        associations.insert(att);
        didFind = true;
        break; // no need to check other attribute resources (attributes are unique across resources).
      }
    }
  }
  return didFind;
}

// A convenience method that calls insertEntityAssociations and returns a newly-constructed set.
std::set<smtk::attribute::AttributePtr> Resource::associations(const EntityRef& modelEntity)
{
  std::set<smtk::attribute::AttributePtr> result;
  this->insertEntityAssociations(modelEntity, result);
  return result;
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
/// Add a vertex to the resource (without any relationships) at the given \a uid.
Vertex Resource::insertVertex(const UUID& uid)
{
  return Vertex(shared_from_this(), this->setEntityOfTypeAndDimension(uid, CELL_ENTITY, 0)->first);
}

/// Add an edge to the resource (without any relationships) at the given \a uid.
Edge Resource::insertEdge(const UUID& uid)
{
  return Edge(shared_from_this(), this->setEntityOfTypeAndDimension(uid, CELL_ENTITY, 1)->first);
}

/// Add a face to the resource (without any relationships) at the given \a uid.
Face Resource::insertFace(const UUID& uid)
{
  return Face(shared_from_this(), this->setEntityOfTypeAndDimension(uid, CELL_ENTITY, 2)->first);
}

/// Add a volume to the resource (without any relationships) at the given \a uid.
Volume Resource::insertVolume(const UUID& uid)
{
  return Volume(shared_from_this(), this->setEntityOfTypeAndDimension(uid, CELL_ENTITY, 3)->first);
}

/// Add an edge to the resource (without any relationships)
Vertex Resource::addVertex()
{
  return Vertex(shared_from_this(), this->addEntityOfTypeAndDimension(CELL_ENTITY, 0));
}

/// Add an edge to the resource (without any relationships)
Edge Resource::addEdge()
{
  return Edge(shared_from_this(), this->addEntityOfTypeAndDimension(CELL_ENTITY, 1));
}

/**\brief Add a face to the resource (without any relationships)
  *
  * While this method does not add any relations, it
  * does create two HAS_USE arrangements to hold
  * FaceUse instances (assuming the Resource may be
  * downcast to a Resource instance).
  */
Face Resource::addFace()
{
  return Face(shared_from_this(), this->addEntityOfTypeAndDimension(CELL_ENTITY, 2));
}

/// Add a volume to the resource (without any relationships)
Volume Resource::addVolume()
{
  return Volume(shared_from_this(), this->addEntityOfTypeAndDimension(CELL_ENTITY, 3));
}

/// Insert a VertexUse at the specified \a uid.
VertexUse Resource::insertVertexUse(const UUID& uid)
{
  return VertexUse(
    shared_from_this(), this->setEntityOfTypeAndDimension(uid, USE_ENTITY, 0)->first);
}

/// Create a VertexUse with the specified \a uid and replace \a src's VertexUse.
VertexUse Resource::setVertexUse(const UUID& uid, const Vertex& src, int sense)
{
  VertexUse vertUse = this->insertVertexUse(uid);
  this->findCreateOrReplaceCellUseOfSenseAndOrientation(src.entity(), sense, POSITIVE, uid);
  return vertUse;
}

/// Insert a EdgeUse at the specified \a uid.
EdgeUse Resource::insertEdgeUse(const UUID& uid)
{
  return EdgeUse(shared_from_this(), this->setEntityOfTypeAndDimension(uid, USE_ENTITY, 1)->first);
}

/// Create a EdgeUse with the specified \a uid and replace \a src's EdgeUse.
EdgeUse Resource::setEdgeUse(const UUID& uid, const Edge& src, int sense, Orientation o)
{
  EdgeUse edgeUse = this->insertEdgeUse(uid);
  this->findCreateOrReplaceCellUseOfSenseAndOrientation(src.entity(), sense, o, uid);
  return edgeUse;
}

/// Insert a FaceUse at the specified \a uid.
FaceUse Resource::insertFaceUse(const UUID& uid)
{
  return FaceUse(shared_from_this(), this->setEntityOfTypeAndDimension(uid, USE_ENTITY, 2)->first);
}

/// Create a FaceUse with the specified \a uid and replace \a src's FaceUse.
FaceUse Resource::setFaceUse(const UUID& uid, const Face& src, int sense, Orientation o)
{
  FaceUse faceUse = this->insertFaceUse(uid);
  this->findCreateOrReplaceCellUseOfSenseAndOrientation(src.entity(), sense, o, uid);
  return faceUse;
}

/// Insert a VolumeUse at the specified \a uid.
VolumeUse Resource::insertVolumeUse(const UUID& uid)
{
  return VolumeUse(
    shared_from_this(), this->setEntityOfTypeAndDimension(uid, USE_ENTITY, 3)->first);
}

/// Create a VolumeUse with the specified \a uid and replace \a src's VolumeUse.
VolumeUse Resource::setVolumeUse(const UUID& uid, const Volume& src)
{
  VolumeUse volUse = this->insertVolumeUse(uid);
  this->findCreateOrReplaceCellUseOfSenseAndOrientation(src.entity(), 0, POSITIVE, uid);
  return volUse;
}

/// Add a vertex-use to the resource (without any relationships)
VertexUse Resource::addVertexUse()
{
  return VertexUse(shared_from_this(), this->addEntityOfTypeAndDimension(USE_ENTITY, 0));
}

/// Find or add a vertex-use to the resource with a relationship back to a vertex.
VertexUse Resource::addVertexUse(const Vertex& src, int sense)
{
  if (src.isValid() && src.resource().get() == this)
  {
    return VertexUse(
      src.resource(),
      this->findCreateOrReplaceCellUseOfSenseAndOrientation(src.entity(), sense, POSITIVE));
  }
  return VertexUse(); // invalid vertex use if source vertex was invalid or from a different resource.
}

/// Add an edge-use to the resource (without any relationships)
EdgeUse Resource::addEdgeUse()
{
  return EdgeUse(shared_from_this(), this->addEntityOfTypeAndDimension(USE_ENTITY, 1));
}

/// Find or add a edge-use to the resource with a relationship back to a edge.
EdgeUse Resource::addEdgeUse(const Edge& src, int sense, Orientation orient)
{
  if (src.isValid() && src.resource().get() == this)
  {
    return EdgeUse(
      src.resource(),
      this->findCreateOrReplaceCellUseOfSenseAndOrientation(src.entity(), sense, orient));
  }
  return EdgeUse(); // invalid edge use if source edge was invalid or from a different resource.
}

/// Add a face-use to the resource (without any relationships)
FaceUse Resource::addFaceUse()
{
  return FaceUse(shared_from_this(), this->addEntityOfTypeAndDimension(USE_ENTITY, 2));
}

/// Find or add a face-use to the resource with a relationship back to a face.
FaceUse Resource::addFaceUse(const Face& src, int sense, Orientation orient)
{
  if (src.isValid() && src.resource().get() == this)
  {
    return FaceUse(
      src.resource(),
      src.resource()->findCreateOrReplaceCellUseOfSenseAndOrientation(src.entity(), sense, orient));
  }
  return FaceUse(); // invalid face use if source face was invalid or from a different resource.
}

/// Add a volume-use to the resource (without any relationships)
VolumeUse Resource::addVolumeUse()
{
  return VolumeUse(shared_from_this(), this->addEntityOfTypeAndDimension(USE_ENTITY, 3));
}

/// Find or add a volume-use to the resource with a relationship back to a volume.
VolumeUse Resource::addVolumeUse(const Volume& src)
{
  if (src.isValid() && src.resource().get() == this)
  {
    return VolumeUse(
      src.resource(),
      src.resource()->findCreateOrReplaceCellUseOfSenseAndOrientation(src.entity(), 0, POSITIVE));
  }
  return VolumeUse(); // invalid volume use if source volume was invalid or from a different resource.
}

/// Insert a Chain at the specified \a uid.
Chain Resource::insertChain(const UUID& uid)
{
  return Chain(
    shared_from_this(),
    this->setEntityOfTypeAndDimension(uid, SHELL_ENTITY | DIMENSION_0 | DIMENSION_1, -1)->first);
}

/// Find or add a chain to the resource with a relationship back to its owning edge-use.
Chain Resource::setChain(const UUID& uid, const EdgeUse& use)
{
  Chain chain = this->insertChain(uid);
  this->findOrAddIncludedShell(use.entity(), uid);
  return chain;
}

/// Find or add a chain to the resource with a relationship back to its owning chain.
Chain Resource::setChain(const UUID& uid, const Chain& parent)
{
  Chain chain = this->insertChain(uid);
  this->findOrAddIncludedShell(parent.entity(), uid);
  return chain;
}

/// Insert a Loop at the specified \a uid.
Loop Resource::insertLoop(const UUID& uid)
{
  return Loop(
    shared_from_this(),
    this->setEntityOfTypeAndDimension(uid, SHELL_ENTITY | DIMENSION_1 | DIMENSION_2, -1)->first);
}

/// Find or add a chain to the resource with a relationship back to its owning face-use.
Loop Resource::setLoop(const UUID& uid, const FaceUse& use)
{
  Loop loop = this->insertLoop(uid);
  this->findOrAddIncludedShell(use.entity(), uid);
  return loop;
}

/// Find or add a chain to the resource with a relationship back to its owning loop.
Loop Resource::setLoop(const UUID& uid, const Loop& parent)
{
  Loop loop = this->insertLoop(uid);
  this->findOrAddIncludedShell(parent.entity(), uid);
  return loop;
}

/// Insert a Shell at the specified \a uid.
Shell Resource::insertShell(const UUID& uid)
{
  return Shell(
    shared_from_this(),
    this->setEntityOfTypeAndDimension(uid, SHELL_ENTITY | DIMENSION_2 | DIMENSION_3, -1)->first);
}

/// Find or add a chain to the resource with a relationship back to its owning volume-use.
Shell Resource::setShell(const UUID& uid, const VolumeUse& use)
{
  Shell shell = this->insertShell(uid);
  this->findOrAddIncludedShell(use.entity(), uid);
  return shell;
}

/// Find or add a chain to the resource with a relationship back to its owning shell.
Shell Resource::setShell(const UUID& uid, const Shell& parent)
{
  Shell shell = this->insertShell(uid);
  this->findOrAddIncludedShell(parent.entity(), uid);
  return shell;
}

/// Add a 0/1-d shell (a vertex chain) to the resource (without any relationships)
Chain Resource::addChain()
{
  return Chain(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(SHELL_ENTITY | DIMENSION_0 | DIMENSION_1, -1));
}

/// Add a 0/1-d shell (a vertex chain) to the resource with a relation to its edge use
Chain Resource::addChain(const EdgeUse& eu)
{
  if (eu.isValid() && eu.resource().get() == this)
  {
    return Chain(eu.resource(), eu.resource()->createIncludedShell(eu.entity()));
  }
  return Chain();
}

/// Add a 0/1-d shell (a vertex chain) to the resource with a relation to its edge use
Chain Resource::addChain(const Chain& c)
{
  if (c.isValid() && c.resource().get() == this)
  {
    return Chain(c.resource(), c.resource()->createIncludedShell(c.entity()));
  }
  return Chain();
}

/// Add a 1/2-d shell (an edge loop) to the resource (without any relationships)
Loop Resource::addLoop()
{
  return Loop(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(SHELL_ENTITY | DIMENSION_1 | DIMENSION_2, -1));
}

/// Add a 1/2-d shell (an edge loop) to the resource with a relation to its parent face use
Loop Resource::addLoop(const FaceUse& fu)
{
  if (fu.isValid() && fu.resource().get() == this)
  {
    return Loop(fu.resource(), fu.resource()->createIncludedShell(fu.entity()));
  }
  return Loop();
}

/// Add a 1/2-d shell (an edge loop) to the resource with a relation to its parent loop
Loop Resource::addLoop(const Loop& lp)
{
  if (lp.isValid() && lp.resource().get() == this)
  {
    return Loop(lp.resource(), lp.resource()->createIncludedShell(lp.entity()));
  }
  return Loop();
}

/// Add a 2/3-d shell (a face-shell) to the resource (without any relationships)
Shell Resource::addShell()
{
  return Shell(
    shared_from_this(),
    this->addEntityOfTypeAndDimension(SHELL_ENTITY | DIMENSION_2 | DIMENSION_3, -1));
}

/// A convenience method to find or create a volume use for the volume plus a shell.
Shell Resource::addShell(const Volume& v)
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

/// Add a 2/3-d shell (an face shell) to the resource with a relation to its volume
Shell Resource::addShell(const VolumeUse& v)
{
  if (v.isValid() && v.resource().get() == this)
  {
    return Shell(v.resource(), v.resource()->createIncludedShell(v.entity()));
  }
  return Shell();
}

/**\brief Add an entity group to the resource (without any relationships).
  *
  * Any non-zero bits set in \a extraFlags are OR'd with entityFlags() of the group.
  * This is an easy way to constrain the entities allowed to be members
  * of the group.
  *
  * You may also specify a \a name for the group. If \a name is empty, then no
  * name is assigned.
  */
Group Resource::insertGroup(const UUID& uid, int extraFlags, const std::string& groupName)
{
  UUIDWithEntityPtr result = this->setEntityOfTypeAndDimension(uid, GROUP_ENTITY | extraFlags, -1);
  if (result == m_topology->end())
    return Group();

  if (!groupName.empty())
    this->setStringProperty(uid, "name", groupName);

  return Group(shared_from_this(), uid);
}

/// Add a group, creating a new UUID in the process. \sa insertGroup().
Group Resource::addGroup(int extraFlags, const std::string& groupName)
{
  UUID uid = this->unusedUUID();
  return this->insertGroup(uid, extraFlags, groupName);
}

/// Add auxiliary geometry (of the given \a dim, which may be -1) to the resource with the specified \a uid.
AuxiliaryGeometry Resource::insertAuxiliaryGeometry(const smtk::common::UUID& uid, int dim)
{
  UUIDWithEntityPtr result = this->setEntityOfTypeAndDimension(uid, AUX_GEOM_ENTITY, dim);
  if (result == m_topology->end())
  {
    return AuxiliaryGeometry();
  }
  return AuxiliaryGeometry(shared_from_this(), uid);
}

/// Add auxiliary geometry (of the given \a dim, which may be -1) to the resource.
AuxiliaryGeometry Resource::addAuxiliaryGeometry(int dim)
{
  UUID uid = this->unusedUUID();
  return this->insertAuxiliaryGeometry(uid, dim);
}

/// Add auxiliary geometry (of the given \a dim, which may be -1) to the resource, embedded in its \a parent.
AuxiliaryGeometry Resource::addAuxiliaryGeometry(const Model& parent, int dim)
{
  Model mutableParent(parent);
  AuxiliaryGeometry auxGeom = this->addAuxiliaryGeometry(dim);
  mutableParent.addAuxiliaryGeometry(auxGeom);
  return auxGeom;
}

/// Add auxiliary geometry (of the given \a dim, which may be -1) to the resource, embedded in its \a parent.
AuxiliaryGeometry Resource::addAuxiliaryGeometry(const AuxiliaryGeometry& parent, int dim)
{
  AuxiliaryGeometry mutableParent(parent);
  AuxiliaryGeometry auxGeom = this->addAuxiliaryGeometry(dim);
  mutableParent.embedEntity(auxGeom);
  return auxGeom;
}

/**\brief Add a model to the resource.
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
Model Resource::insertModel(
  const UUID& uid,
  int parametricDim,
  int embeddingDim,
  const std::string& modelName)
{
  UUIDWithEntityPtr result = this->setEntityOfTypeAndDimension(uid, MODEL_ENTITY, parametricDim);
  if (result == m_topology->end())
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
Model Resource::addModel(int parametricDim, int embeddingDim, const std::string& modelName)
{
  UUID uid = this->unusedUUID();
  return this->insertModel(uid, parametricDim, embeddingDim, modelName);
}

/**\brief Add an instance of some model entity to the resource.
  *
  * An instance is a reference to some other item in the resource.
  * Any entity may be instanced, but generally models are instanced
  * as part of a scene graph.
  */
Instance Resource::addInstance()
{
  UUID uid = this->addEntityOfTypeAndDimension(INSTANCE_ENTITY, -1);
  return Instance(shared_from_this(), uid);
}

/**\brief Add an instance with the given prototype to the resource.
  *
  * The prototype \a object (the parent of the instance)
  * is a reference to some other item in the resource.
  * Any entity may be instanced, but generally models are instanced
  * as part of a scene graph.
  */
Instance Resource::addInstance(const EntityRef& object)
{
  if (object.isValid())
  {
    UUID uid = this->addEntityOfTypeAndDimension(INSTANCE_ENTITY, -1);
    int iidx = this->findEntity(object.entity())->findOrAppendRelation(uid);
    int oidx = this->findEntity(uid)->findOrAppendRelation(object.entity());
    this->arrangeEntity(uid, INSTANCE_OF, Arrangement::InstanceInstanceOfWithIndex(oidx));
    this->arrangeEntity(
      object.entity(), INSTANCED_BY, Arrangement::InstanceInstanceOfWithIndex(iidx));
    return Instance(shared_from_this(), uid);
  }
  return Instance();
}
//@}

/**\brief Unregister a session from the model resource.
  *
  */
bool Resource::closeSession(const SessionRef& sref)
{
  if (sref.resource().get() == this)
  {
    UUIDsToSessions::iterator us = m_sessions->find(sref.entity());
    if (us != m_sessions->end())
    {
      EntityRefs all; // a set of all entities in the session
      Models models = sref.models<Models>();
      EntityIterator eit;
      eit.traverse(models.begin(), models.end(), ITERATE_MODELS);
      for (eit.begin(); !eit.isAtEnd(); ++eit)
      {
        if (!eit->isSessionRef())
        {
          all.insert(*eit);
        }
      }
      for (const auto& ent : all)
      {
        this->hardErase(ent);
      }
      bool didClose = this->unregisterSession(sref.session(), true);
      return didClose;
    }
  }
  else
  {
    smtkErrorMacro(
      this->log(),
      "Asked to close session (" << sref.name() << ") "
                                 << "owned by a different model resource (" << sref.resource().get()
                                 << " vs " << this << ")!");
  }
  return false;
}

/**\brief Return an array of all the sessions this resource owns.
  *
  */
SessionRefs Resource::sessions() const
{
  SessionRefs result;
  UUIDsToSessions::const_iterator it;
  for (it = m_sessions->begin(); it != m_sessions->end(); ++it)
    result.push_back(SessionRef(smtk::const_pointer_cast<Resource>(shared_from_this()), it->first));
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
/// Request notification from this resource instance when \a event occurs.
void Resource::observe(ResourceEventType event, ConditionCallback functionHandle, void* callData)
{
  if (event.first == ANY_EVENT)
  {
    int i;
    int iend = static_cast<int>(ANY_EVENT);
    for (i = static_cast<int>(ADD_EVENT); i != iend; ++i)
    {
      event.first = static_cast<ResourceEventChangeType>(i);
      this->observe(event, functionHandle, callData);
    }

    return;
  }

  m_conditionTriggers.insert(ConditionTrigger(event, ConditionObserver(functionHandle, callData)));
}

/// Request notification from this resource instance when \a event occurs.
void Resource::observe(ResourceEventType event, OneToOneCallback functionHandle, void* callData)
{
  if (event.first == ANY_EVENT)
  {
    int i;
    int iend = static_cast<int>(ANY_EVENT);
    for (i = static_cast<int>(ADD_EVENT); i != iend; ++i)
    {
      event.first = static_cast<ResourceEventChangeType>(i);
      this->observe(event, functionHandle, callData);
    }

    return;
  }

  m_oneToOneTriggers.insert(OneToOneTrigger(event, OneToOneObserver(functionHandle, callData)));
}

/// Request notification from this resource instance when \a event occurs.
void Resource::observe(ResourceEventType event, OneToManyCallback functionHandle, void* callData)
{
  if (event.first == ANY_EVENT)
  {
    int i;
    int iend = static_cast<int>(ANY_EVENT);
    for (i = static_cast<int>(ADD_EVENT); i != iend; ++i)
    {
      event.first = static_cast<ResourceEventChangeType>(i);
      this->observe(event, functionHandle, callData);
    }

    return;
  }

  m_oneToManyTriggers.insert(OneToManyTrigger(event, OneToManyObserver(functionHandle, callData)));
}

/// Decline further notification from this resource instance when \a event occurs.
void Resource::unobserve(ResourceEventType event, ConditionCallback functionHandle, void* callData)
{
  if (event.first == ANY_EVENT)
  {
    int i;
    int iend = static_cast<int>(ANY_EVENT);
    for (i = static_cast<int>(ADD_EVENT); i != iend; ++i)
    {
      event.first = static_cast<ResourceEventChangeType>(i);
      this->unobserve(event, functionHandle, callData);
    }

    return;
  }

  m_conditionTriggers.erase(ConditionTrigger(event, ConditionObserver(functionHandle, callData)));
}

/// Decline further notification from this resource instance when \a event occurs.
void Resource::unobserve(ResourceEventType event, OneToOneCallback functionHandle, void* callData)
{
  if (event.first == ANY_EVENT)
  {
    int i;
    int iend = static_cast<int>(ANY_EVENT);
    for (i = static_cast<int>(ADD_EVENT); i != iend; ++i)
    {
      event.first = static_cast<ResourceEventChangeType>(i);
      this->unobserve(event, functionHandle, callData);
    }

    return;
  }

  m_oneToOneTriggers.erase(OneToOneTrigger(event, OneToOneObserver(functionHandle, callData)));
}

/// Decline further notification from this resource instance when \a event occurs.
void Resource::unobserve(ResourceEventType event, OneToManyCallback functionHandle, void* callData)
{
  if (event.first == ANY_EVENT)
  {
    int i;
    int iend = static_cast<int>(ANY_EVENT);
    for (i = static_cast<int>(ADD_EVENT); i != iend; ++i)
    {
      event.first = static_cast<ResourceEventChangeType>(i);
      this->unobserve(event, functionHandle, callData);
    }

    return;
  }

  m_oneToManyTriggers.erase(OneToManyTrigger(event, OneToManyObserver(functionHandle, callData)));
}

/// Called by this Resource instance or EntityRef instances referencing it when \a event occurs.
void Resource::trigger(ResourceEventType event, const smtk::model::EntityRef& src)
{
  std::set<ConditionTrigger>::const_iterator begin = m_conditionTriggers.lower_bound(
    ConditionTrigger(event, ConditionObserver(ConditionCallback(), static_cast<void*>(nullptr))));
  std::set<ConditionTrigger>::const_iterator end = m_conditionTriggers.upper_bound(ConditionTrigger(
    std::make_pair(event.first, static_cast<ResourceEventRelationType>(event.second + 1)),
    ConditionObserver(ConditionCallback(), static_cast<void*>(nullptr))));
  for (std::set<ConditionTrigger>::const_iterator it = begin; it != end; ++it)
    (*it->second.first)(it->first, src, it->second.second);
}

/// Called by this Resource instance or EntityRef instances referencing it when \a event occurs.
void Resource::trigger(
  ResourceEventType event,
  const smtk::model::EntityRef& src,
  const smtk::model::EntityRef& related)
{
  std::set<OneToOneTrigger>::const_iterator begin = m_oneToOneTriggers.lower_bound(
    OneToOneTrigger(event, OneToOneObserver(OneToOneCallback(), static_cast<void*>(nullptr))));
  std::set<OneToOneTrigger>::const_iterator end = m_oneToOneTriggers.upper_bound(OneToOneTrigger(
    std::make_pair(event.first, static_cast<ResourceEventRelationType>(event.second + 1)),
    OneToOneObserver(OneToOneCallback(), static_cast<void*>(nullptr))));
  for (std::set<OneToOneTrigger>::const_iterator it = begin; it != end; ++it)
    (*it->second.first)(it->first, src, related, it->second.second);
}

/// Called by this Resource instance or EntityRef instances referencing it when \a event occurs.
void Resource::trigger(
  ResourceEventType event,
  const smtk::model::EntityRef& src,
  const smtk::model::EntityRefArray& related)
{
  std::set<OneToManyTrigger>::const_iterator begin = m_oneToManyTriggers.lower_bound(
    OneToManyTrigger(event, OneToManyObserver(OneToManyCallback(), static_cast<void*>(nullptr))));
  std::set<OneToManyTrigger>::const_iterator end = m_oneToManyTriggers.upper_bound(OneToManyTrigger(
    std::make_pair(event.first, static_cast<ResourceEventRelationType>(event.second + 1)),
    OneToManyObserver(OneToManyCallback(), static_cast<void*>(nullptr))));
  for (std::set<OneToManyTrigger>::const_iterator it = begin; it != end; ++it)
    (*it->second.first)(it->first, src, related, it->second.second);
}

template<typename T>
std::string uniqueResourceName(EntityRef ent, const T& preexisting, int& counter)
{
  std::string prefix = ent.isModel() ? "model" : "auxgeo";
  std::string result;
  do
  {
    std::ostringstream ns;
    ns << prefix << (++counter);
    result = ns.str();
  } while (preexisting.find(result) != preexisting.end());
  return result;
}

/// Internal method used by modelOwningEntity to prevent infinite recursion
UUID Resource::modelOwningEntityRecursive(const UUID& ent, std::set<UUID>& visited) const
{
  if (visited.find(ent) != visited.end())
  {
    return UUID::null(); // We've already search here with no result.
  }
  visited.insert(ent);

  UUID uid(ent);
  UUIDWithConstEntityPtr it = m_topology->find(uid);
  if (it != m_topology->end())
  {
    // If we have a use or a shell, get the associated cell, if any
    smtk::model::BitFlags etype = it->second->entityFlags();
    switch (etype & ENTITY_MASK)
    {
      case AUX_GEOM_ENTITY:
      {
        ResourcePtr self = const_cast<Resource*>(this)->shared_from_this();
        EntityRef aux(self, ent);
        for (aux = aux.embeddedIn(); aux.isValid(); aux = aux.embeddedIn())
        {
          if (aux.isModel())
          {
            return aux.entity();
          }
        }
        return UUID();
      }
      break;
      case GROUP_ENTITY:
      {
        // If we have a superset arrangement, ask for supersets and traverse upwards.
        ResourcePtr self = const_cast<Resource*>(this)->shared_from_this();
        EntityRefArray supersets;
        EntityRefArrangementOps::appendAllRelations(
          EntityRef(self->shared_from_this(), ent), SUBSET_OF, supersets);
        if (!supersets.empty())
        {
          EntityRef super;
          for (EntityRefArray::iterator spit = supersets.begin(); spit != supersets.end(); ++spit)
          {
            if (spit->isModel())
              return spit->entity();
          }
          // No models are in my superset... traverse up the first superset to find a model:
          for (auto ssentry = supersets.begin(); ssentry != supersets.end(); ++ssentry)
          {
            UUID result = this->modelOwningEntityRecursive(ssentry->entity(), visited);
            if (result)
            {
              return result;
            }
          }
        }

        // Assume the first relationship that is a group or model is our owner.
        // Keep going up parent groups until we hit the top.
        for (UUIDArray::const_iterator sit = it->second->relations().begin();
             sit != it->second->relations().end();
             ++sit)
        {
          UUIDWithConstEntityPtr subentity = this->topology().find(*sit);
          if (subentity != this->topology().end() && subentity->first != uid)
          {
            if (subentity->second->entityFlags() & MODEL_ENTITY)
              return subentity->first;
            if (subentity->second->entityFlags() & GROUP_ENTITY)
            { // Switch to finding relations of the group (assume it is our parent)
              uid = subentity->first;
              it = m_topology->find(uid);
              sit = it->second->relations().begin();
            }
          }
        }
      }
      break;
      case INSTANCE_ENTITY:
        // Look for any relationship. We assume the first one is our prototype.
        for (UUIDArray::const_iterator sit = it->second->relations().begin();
             sit != it->second->relations().end();
             ++sit)
        {
          UUIDWithConstEntityPtr subentity = this->topology().find(*sit);
          if (subentity != this->topology().end() && subentity->first != uid)
          {
            if (subentity->second->entityFlags() & MODEL_ENTITY)
              return subentity->first;
            UUID result = this->modelOwningEntityRecursive(subentity->first, visited);
            if (result)
            {
              return result;
            }
            break;
          }
        }
        break;
      case SHELL_ENTITY:
        // Loop for a relationship to a use.
        for (UUIDArray::const_iterator sit = it->second->relations().begin();
             sit != it->second->relations().end();
             ++sit)
        {
          UUIDWithConstEntityPtr subentity = this->topology().find(*sit);
          if (
            subentity != this->topology().end() &&
            smtk::model::isUseEntity(subentity->second->entityFlags()))
          {
            it = subentity;
            break;
          }
        }
      // Now fall through and look for the use's relationship to a cell.
      // fall through
      case USE_ENTITY:
        // Look for a relationship to a cell
        for (UUIDArray::const_iterator sit = it->second->relations().begin();
             sit != it->second->relations().end();
             ++sit)
        {
          UUIDWithConstEntityPtr subentity = this->topology().find(*sit);
          if (
            subentity != this->topology().end() &&
            smtk::model::isCellEntity(subentity->second->entityFlags()))
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
        // we attempt to cast ourselves to Resource and identify a parent model.
        {
          // Although const_pointer_cast is evil, changing the entityref classes
          // to accept any type of shared_ptr<X/X const> is more evil.
          ResourcePtr self = smtk::const_pointer_cast<Resource>(shared_from_this());
          return Model(self, ent).parent().entity();
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
    for (dim = it->second->dimension(); dim >= 0 && dim < 4; ++dim)
    {
      for (UUIDs::iterator uit = uids.begin(); uit != uids.end(); ++uit)
      {
        EntityPtr bordEnt = this->findEntity(*uit);
        if (!bordEnt)
          continue;
        for (UUIDArray::const_iterator rit = bordEnt->relations().begin();
             rit != bordEnt->relations().end();
             ++rit)
        {
          EntityPtr relEnt = this->findEntity(*rit);
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

/// Attempt to find a session owning the given entity.
UUID Resource::sessionOwningEntityRecursive(const UUID& ent, std::set<UUID>& visited) const
{
  if (visited.find(ent) != visited.end())
  {
    return UUID::null(); // We've already been here with no result
  }
  visited.insert(ent);

  EntityPtr erec = this->findEntity(ent);
  if (erec)
  {
    // Traverse up to the owning model
    UUID uid = isModel(erec->entityFlags()) ? ent : modelOwningEntity(ent);
    // The parent of a model should be another model or session.

    // If we have a superset arrangement, ask for supersets and traverse upwards.
    ResourcePtr self = const_cast<Resource*>(this)->shared_from_this();
    EntityRefArray supersets;
    EntityRefArrangementOps::appendAllRelations(
      EntityRef(self->shared_from_this(), uid), SUBSET_OF, supersets);
    if (!supersets.empty())
    {
      EntityRef super;
      for (EntityRefArray::iterator spit = supersets.begin(); spit != supersets.end(); ++spit)
      {
        if (spit->isSessionRef())
          return spit->entity();
      }
      // No sessions are in my superset... traverse up the first superset to find one:
      for (auto ssentry = supersets.begin(); ssentry != supersets.end(); ++ssentry)
      {
        UUID result = this->sessionOwningEntityRecursive(ssentry->entity(), visited);
        if (result)
        {
          return result;
        }
      }
    }

    // Assume the first relationship that is a session or model is our owner.
    // Keep going up parents until we hit the top.
    UUIDWithConstEntityPtr it = m_topology->find(uid);
    for (UUIDArray::const_iterator sit = it->second->relations().begin();
         sit != it->second->relations().end();
         ++sit)
    {
      UUIDWithConstEntityPtr subentity = this->topology().find(*sit);
      if (subentity != this->topology().end() && subentity->first != uid)
      {
        if (isSessionRef(subentity->second->entityFlags()))
          return subentity->first;
        if (isModel(subentity->second->entityFlags()))
        { // Switch to finding relations of the model (assume it is our parent)
          uid = subentity->first;
          it = m_topology->find(uid);
          sit = it->second->relations().begin();
        }
      }
    }
    return uid;
  }
  return UUID::null();
}

} // namespace model
} //namespace smtk
