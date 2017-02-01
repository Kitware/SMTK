//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Manager_h
#define __smtk_model_Manager_h

#include "smtk/Options.h" // for SMTK_HASH_STORAGE
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"
#include "smtk/SharedPtr.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Arrangement.h"
#include "smtk/model/AttributeAssignments.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Events.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/PropertyType.h"
#include "smtk/model/Session.h"
#include "smtk/model/SessionRef.h"
#include "smtk/model/StringData.h"
#include "smtk/model/Tessellation.h"

#include "smtk/common/UUID.h"
#include "smtk/common/UUIDGenerator.h"

#include "smtk/io/Logger.h"

#ifdef SMTK_HASH_STORAGE
#  if defined(_MSC_VER) // Visual studio
#    pragma warning (push)
#    pragma warning (disable : 4996)  // Overeager "unsafe" parameter check
#  endif
#  include "sparsehash/sparse_hash_map"
#  if defined(_MSC_VER) // Visual studio
#    pragma warning (pop)
#  endif
#else
#  include <map>
#endif

#include <algorithm>
#include <set>
#include <map>
#include <vector>

#include <sstream>

/**\brief The name of an integer property used to store display Tessellation generation numbers.
  *
  * Starting with "_" indicates internal-use-only.
  * Short (8 bytes or less) means single word comparison suffices on many platforms => fast.
  */
#define SMTK_TESS_GEN_PROP "_tessgen"

/**\brief The name of an integer property used to store mesh Tessellation generation numbers.
  *
  * \sa SMTK_MESH_GEN_PROP
  */
#define SMTK_MESH_GEN_PROP "_meshgen"

/**\brief The name of an integer property used to store the geometric representation style of a model.
  *
  * \sa ModelGeometryStyle
  */
#define SMTK_GEOM_STYLE_PROP "_geomstyle"

/**\brief The name of an float property used to store the bounding box of a model entity
  *
  * \sa  SMTK_BOUNDING_BOX_PROP
  */
#define SMTK_BOUNDING_BOX_PROP "_boundingBox"

namespace smtk {
  namespace model {

#ifdef SMTK_HASH_STORAGE
/// Store information mapping IDs to Entity records. This is the primary storage for SMTK models.
typedef google::sparse_hash_map<smtk::common::UUID,Entity> UUIDsToEntities;
#else
/// Store information mapping IDs to Entity records. This is the primary storage for SMTK models.
typedef std::map<smtk::common::UUID,Entity> UUIDsToEntities;
#endif

/// An abbreviation for an iterator into primary model storage.
typedef UUIDsToEntities::iterator UUIDWithEntity;
/**\brief Store information about solid models.
  *
  * This adds information about arrangements and tessellations
  * of entities to its Manager base class.
  */
class SMTKCORE_EXPORT Manager : smtkEnableSharedPtr(Manager)
{
public:
  typedef UUIDsToEntities storage_type;
  typedef storage_type::iterator iter_type;
  typedef UUIDsToTessellations::iterator tess_iter_type;

  smtkTypeMacro(Manager);
  smtkCreateMacro(Manager);
  Manager();
  Manager(
    shared_ptr<UUIDsToEntities> topology,
    shared_ptr<UUIDsToArrangements> arrangements,
    shared_ptr<UUIDsToTessellations> tess,
    shared_ptr<UUIDsToTessellations> analysismesh,
    shared_ptr<smtk::mesh::Manager>  meshes,
    shared_ptr<UUIDsToAttributeAssignments> attribs);
  virtual ~Manager();

  UUIDsToEntities& topology();
  const UUIDsToEntities& topology() const;

  UUIDsToArrangements& arrangements();
  const UUIDsToArrangements& arrangements() const;

  UUIDsToTessellations& tessellations();
  const UUIDsToTessellations& tessellations() const;

  UUIDsToTessellations& analysisMesh();
  const UUIDsToTessellations& analysisMesh() const;

  smtk::mesh::ManagerPtr meshes() const;

  const UUIDsToAttributeAssignments& attributeAssignments() const;

  BitFlags type(const smtk::common::UUID& ofEntity) const;
  int dimension(const smtk::common::UUID& ofEntity) const;
  std::string name(const smtk::common::UUID& ofEntity) const;

  const Entity* findEntity(const smtk::common::UUID& uid, bool trySessions = true) const;
  Entity* findEntity(const smtk::common::UUID& uid, bool trySessions = true);

  virtual SessionInfoBits erase(const smtk::common::UUID& uid, SessionInfoBits flags = smtk::model::SESSION_EVERYTHING);
  virtual SessionInfoBits erase(const EntityRef& entityref, SessionInfoBits flags = smtk::model::SESSION_EVERYTHING);
  virtual SessionInfoBits eraseModel(const Model& entityref, SessionInfoBits flags = smtk::model::SESSION_EVERYTHING);

  smtk::common::UUIDs bordantEntities(const smtk::common::UUID& ofEntity, int ofDimension = -2) const;
  smtk::common::UUIDs bordantEntities(const smtk::common::UUIDs& ofEntities, int ofDimension = -2) const;
  smtk::common::UUIDs boundaryEntities(const smtk::common::UUID& ofEntity, int ofDimension = -2) const;
  smtk::common::UUIDs boundaryEntities(const smtk::common::UUIDs& ofEntities, int ofDimension = -2) const;

  smtk::common::UUIDs lowerDimensionalBoundaries(const smtk::common::UUID& ofEntity, int lowerDimension);
  smtk::common::UUIDs higherDimensionalBordants(const smtk::common::UUID& ofEntity, int higherDimension);
  smtk::common::UUIDs adjacentEntities(const smtk::common::UUID& ofEntity, int ofDimension);

  smtk::common::UUIDs entitiesMatchingFlags(BitFlags mask, bool exactMatch = true);
  smtk::common::UUIDs entitiesOfDimension(int dim);

  smtk::common::UUID unusedUUID();
  iter_type insertEntityOfTypeAndDimension(BitFlags entityFlags, int dim);
  iter_type insertEntity(Entity& cell);
  iter_type setEntityOfTypeAndDimension(const smtk::common::UUID& uid, BitFlags entityFlags, int dim);
  iter_type setEntity(const smtk::common::UUID& uid, Entity& cell);

  smtk::common::UUID addEntityOfTypeAndDimension(BitFlags entityFlags, int dim);
  smtk::common::UUID addEntity(Entity& cell);
  smtk::common::UUID addEntityOfTypeAndDimensionWithUUID(const smtk::common::UUID& uid, BitFlags entityFlags, int dim);
  smtk::common::UUID addEntityWithUUID(const smtk::common::UUID& uid, Entity& cell);

  iter_type insertCellOfDimension(int dim);
  iter_type setCellOfDimension(const smtk::common::UUID& uid, int dim);
  smtk::common::UUID addCellOfDimension(int dim);
  smtk::common::UUID addCellOfDimensionWithUUID(const smtk::common::UUID& uid, int dim);

  void insertEntityReferences(const UUIDWithEntity& c);
  bool elideOneEntityReference(const UUIDWithEntity& c, const smtk::common::UUID& r);
  void elideEntityReferences(const UUIDWithEntity& c);
  void removeEntityReferences(const UUIDWithEntity& c);

  virtual void addToGroup(const smtk::common::UUID& groupId, const smtk::common::UUIDs& uids);

  void setFloatProperty(const smtk::common::UUID& entity, const std::string& propName, smtk::model::Float propValue);
  void setFloatProperty(const smtk::common::UUID& entity, const std::string& propName, const smtk::model::FloatList& propValue);
  smtk::model::FloatList const& floatProperty(const smtk::common::UUID& entity, const std::string& propName) const;
  smtk::model::FloatList& floatProperty(const smtk::common::UUID& entity, const std::string& propName);
  bool hasFloatProperty(const smtk::common::UUID& entity, const std::string& propName) const;
  bool removeFloatProperty(const smtk::common::UUID& entity, const std::string& propName);
  const UUIDWithFloatProperties floatPropertiesForEntity(const smtk::common::UUID& entity) const;
  UUIDWithFloatProperties floatPropertiesForEntity(const smtk::common::UUID& entity);
  UUIDsToFloatData& floatProperties() { return *this->m_floatData; }
  UUIDsToFloatData const& floatProperties() const { return *this->m_floatData; }

  void setStringProperty(const smtk::common::UUID& entity, const std::string& propName, const smtk::model::String& propValue);
  void setStringProperty(const smtk::common::UUID& entity, const std::string& propName, const smtk::model::StringList& propValue);
  smtk::model::StringList const& stringProperty(const smtk::common::UUID& entity, const std::string& propName) const;
  smtk::model::StringList& stringProperty(const smtk::common::UUID& entity, const std::string& propName);
  bool hasStringProperty(const smtk::common::UUID& entity, const std::string& propName) const;
  bool removeStringProperty(const smtk::common::UUID& entity, const std::string& propName);
  const UUIDWithStringProperties stringPropertiesForEntity(const smtk::common::UUID& entity) const;
  UUIDWithStringProperties stringPropertiesForEntity(const smtk::common::UUID& entity);
  UUIDsToStringData& stringProperties() { return *this->m_stringData; }
  UUIDsToStringData const& stringProperties() const { return *this->m_stringData; }

  void setIntegerProperty(const smtk::common::UUID& entity, const std::string& propName, smtk::model::Integer propValue);
  void setIntegerProperty(const smtk::common::UUID& entity, const std::string& propName, const smtk::model::IntegerList& propValue);
  smtk::model::IntegerList const& integerProperty(const smtk::common::UUID& entity, const std::string& propName) const;
  smtk::model::IntegerList& integerProperty(const smtk::common::UUID& entity, const std::string& propName);
  bool hasIntegerProperty(const smtk::common::UUID& entity, const std::string& propName) const;
  bool removeIntegerProperty(const smtk::common::UUID& entity, const std::string& propName);
  const UUIDWithIntegerProperties integerPropertiesForEntity(const smtk::common::UUID& entity) const;
  UUIDWithIntegerProperties integerPropertiesForEntity(const smtk::common::UUID& entity);
  UUIDsToIntegerData& integerProperties() { return *this->m_integerData; }
  UUIDsToIntegerData const& integerProperties() const { return *this->m_integerData; }

  smtk::common::UUID modelOwningEntity(const smtk::common::UUID& uid) const;
  smtk::common::UUID sessionOwningEntity(const smtk::common::UUID& uid) const;

  void assignDefaultNames();
  void assignDefaultNamesToModelChildren(const smtk::common::UUID& modelId);
  std::string assignDefaultName(const smtk::common::UUID& uid);
  std::string assignDefaultNameIfMissing(const smtk::common::UUID& uid);
  static std::string shortUUIDName(const smtk::common::UUID& uid, BitFlags entityFlags);

  static StringList sessionTypeNames();
  static StringData sessionFileTypes(const std::string& sname, const std::string& engine = std::string());
  static SessionPtr createSessionOfType(const std::string& sname);
  SessionRef createSession(const std::string& sname);
  SessionRef createSession(
    const std::string& sname,
    const smtk::model::SessionRef& sessionIdSpecifier);
  void closeSession(const SessionRef& sess);

  SessionRef registerSession(SessionPtr session);
  bool unregisterSession(SessionPtr session, bool expungeSession = true);
  SessionPtr sessionData(const smtk::model::SessionRef& sessRef) const;
  SessionRefs sessions() const;

  EntityRefArray findEntitiesByProperty(const std::string& pname, Integer pval);
  EntityRefArray findEntitiesByProperty(const std::string& pname, Float pval);
  EntityRefArray findEntitiesByProperty(const std::string& pname, const std::string& pval);
  EntityRefArray findEntitiesByProperty(const std::string& pname, const IntegerList& pval);
  EntityRefArray findEntitiesByProperty(const std::string& pname, const FloatList& pval);
  EntityRefArray findEntitiesByProperty(const std::string& pname, const StringList& pval);
  EntityRefArray findEntitiesOfType(BitFlags flags, bool exactMatch = true);

  template<typename Collection>
  Collection findEntitiesByPropertyAs(const std::string& pname, Integer pval);
  template<typename Collection>
  Collection findEntitiesByPropertyAs(const std::string& pname, const IntegerList& pval);
  template<typename Collection>
  Collection findEntitiesByPropertyAs(const std::string& pname, Float pval);
  template<typename Collection>
  Collection findEntitiesByPropertyAs(const std::string& pname, const FloatList& pval);
  template<typename Collection>
  Collection findEntitiesByPropertyAs(const std::string& pname, const std::string& pval);
  template<typename Collection>
  Collection findEntitiesByPropertyAs(const std::string& pname, const StringList& pval);
  template<typename Collection>
  Collection entitiesMatchingFlagsAs(BitFlags flags, bool exactMatch = true);

  tess_iter_type setTessellation(
    const smtk::common::UUID& cellId,
    const Tessellation& geom,
    int analysis = 0,
    int* gen = NULL);

  tess_iter_type setTessellationAndBoundingBox(
    const smtk::common::UUID& cellId,
    const Tessellation& geom,
    int analysis = 0,
    int* gen = NULL);
  // if BBox is provided, just replace corrds with BBox else we would loop through coords
  // BBox format: [xmin, xmax, ymin, ymax, zmin, zmax]
  bool setBoundingBox(const smtk::common::UUID& cellId,
                     const std::vector<double> & coords, int providedBBox = 0);
  bool removeTessellation(const smtk::common::UUID& cellId, bool removeGen = false);

  int arrangeEntity(const smtk::common::UUID& entityId, ArrangementKind, const Arrangement& arr, int index = -1);
  int unarrangeEntity(const smtk::common::UUID& entityId, ArrangementKind, int index, bool removeIfLast = false);
  bool clearArrangements(const smtk::common::UUID& entityId);

  const Arrangements* hasArrangementsOfKindForEntity(
    const smtk::common::UUID& cellId,
    ArrangementKind) const;
  Arrangements* hasArrangementsOfKindForEntity(
    const smtk::common::UUID& cellId,
    ArrangementKind);

  Arrangements& arrangementsOfKindForEntity(const smtk::common::UUID& cellId, ArrangementKind);

  const Arrangement* findArrangement(const smtk::common::UUID& entityId, ArrangementKind kind, int index) const;
  Arrangement* findArrangement(const smtk::common::UUID& entityId, ArrangementKind kind, int index);
  int findArrangementInvolvingEntity(
    const smtk::common::UUID& entityId, ArrangementKind kind,
    const smtk::common::UUID& involved) const;
  bool findDualArrangements(
    const smtk::common::UUID& entityId, ArrangementKind kind, int index,
    ArrangementReferences& duals) const;
  bool addDualArrangement(
    const smtk::common::UUID& parent, const smtk::common::UUID& child,
    ArrangementKind kind, int sense, Orientation orientation);

  int findCellHasUseWithSense(const smtk::common::UUID& cellId, const smtk::common::UUID& use, int sense) const;
  std::set<int> findCellHasUsesWithOrientation(const smtk::common::UUID& cellId, Orientation orient) const;

  smtk::common::UUID cellHasUseOfSenseAndOrientation(const smtk::common::UUID& cell, int sense, Orientation o) const;
  smtk::common::UUID findCreateOrReplaceCellUseOfSenseAndOrientation(
    const smtk::common::UUID& cell, int sense, Orientation o,
    const smtk::common::UUID& replacement = smtk::common::UUID::null());

  smtk::common::UUIDs useOrShellIncludesShells(const smtk::common::UUID& cellUseOrShell) const;
  smtk::common::UUID createIncludedShell(const smtk::common::UUID& cellUseOrShell);
  bool findOrAddIncludedShell(const smtk::common::UUID& parentUseOrShell, const smtk::common::UUID& shellToInclude);

  //bool shellHasUse(const smtk::common::UUID& shell, const smtk::common::UUID& use) const;
  //smtk::common::UUIDs shellHasUses(const smtk::common::UUID& shell) const;
  bool findOrAddUseToShell(const smtk::common::UUID& shell, const smtk::common::UUID& use);

  bool findOrAddInclusionToCellOrModel(const smtk::common::UUID& cell, const smtk::common::UUID& inclusion);

  bool findOrAddEntityToGroup(const smtk::common::UUID& grp, const smtk::common::UUID& ent);

  bool hasAttribute(const smtk::common::UUID&  attribId, const smtk::common::UUID& toEntity);
  bool associateAttribute(smtk::attribute::System* sys,
                          const smtk::common::UUID&  attribId, const smtk::common::UUID& toEntity);
  bool disassociateAttribute(smtk::attribute::System* sys,
                             const smtk::common::UUID&  attribId, const smtk::common::UUID& fromEntity, bool reverse = true);

  Vertex insertVertex(const smtk::common::UUID& uid);
  Edge insertEdge(const smtk::common::UUID& uid);
  Face insertFace(const smtk::common::UUID& uid);
  Volume insertVolume(const smtk::common::UUID& uid);

  Vertex addVertex();
  Edge addEdge();
  Face addFace();
  Volume addVolume();

  VertexUse insertVertexUse(const smtk::common::UUID& uid);
  VertexUse setVertexUse(const smtk::common::UUID& uid, const Vertex& src, int sense);
  EdgeUse insertEdgeUse(const smtk::common::UUID& uid);
  EdgeUse setEdgeUse(const smtk::common::UUID& uid, const Edge& src, int sense, Orientation o);
  FaceUse insertFaceUse(const smtk::common::UUID& uid);
  FaceUse setFaceUse(const smtk::common::UUID& uid, const Face& src, int sense, Orientation o);
  VolumeUse insertVolumeUse(const smtk::common::UUID& uid);
  VolumeUse setVolumeUse(const smtk::common::UUID& uid, const Volume& src);

  VertexUse addVertexUse();
  VertexUse addVertexUse(const Vertex& src, int sense);
  EdgeUse addEdgeUse();
  EdgeUse addEdgeUse(const Edge& src, int sense, Orientation o);
  FaceUse addFaceUse();
  FaceUse addFaceUse(const Face& src, int sense, Orientation o);
  VolumeUse addVolumeUse();
  VolumeUse addVolumeUse(const Volume& src);

  Chain insertChain(const smtk::common::UUID& uid);
  Chain setChain(const smtk::common::UUID& uid, const EdgeUse& use);
  Chain setChain(const smtk::common::UUID& uid, const Chain& parent);
  Loop insertLoop(const smtk::common::UUID& uid);
  Loop setLoop(const smtk::common::UUID& uid, const FaceUse& use);
  Loop setLoop(const smtk::common::UUID& uid, const Loop& parent);
  Shell insertShell(const smtk::common::UUID& uid);
  Shell setShell(const smtk::common::UUID& uid, const VolumeUse& use);
  Shell setShell(const smtk::common::UUID& uid, const Shell& parent);

  Chain addChain();
  Chain addChain(const EdgeUse&);
  Chain addChain(const Chain&);
  Loop addLoop();
  Loop addLoop(const FaceUse&);
  Loop addLoop(const Loop&);
  Shell addShell();
  Shell addShell(const Volume& src);
  Shell addShell(const VolumeUse& src);

  Group insertGroup(
    const smtk::common::UUID& uid,
    int extraFlags = 0,
    const std::string& name = std::string());
  Group addGroup(int extraFlags = 0, const std::string& name = std::string());

  AuxiliaryGeometry insertAuxiliaryGeometry(const smtk::common::UUID& uid, int dim = -1);
  AuxiliaryGeometry addAuxiliaryGeometry(int dim = -1);
  AuxiliaryGeometry addAuxiliaryGeometry(const Model& parent, int dim = -1);
  AuxiliaryGeometry addAuxiliaryGeometry(const AuxiliaryGeometry& parent, int dim = -1);

  Model insertModel(
    const smtk::common::UUID& uid,
    int parametricDim = 3,
    int embeddingDim = 3,
    const std::string& name = std::string());
  Model addModel(
    int parametricDim = 3, int embeddingDim = 3, const std::string& name = std::string());

  Instance addInstance();
  Instance addInstance(const EntityRef& instanceOf);

  template<typename T>
  bool insertModelFaceWithOrientedOuterLoop(
    const smtk::common::UUID& faceId, // to be created
    const smtk::common::UUID& faceUseId, // to be created
    const smtk::common::UUID& loopId, // to be created
    const T& orderedEdgesWithOrientation);

  template<typename T>
  bool insertModelFaceOrientedInnerLoop(
    const smtk::common::UUID& loopId, // to be created
    const smtk::common::UUID& preExistingLoopId, // the outer loop to contain the inner loop
    const T& orderedEdgesWithOrientation);

  template<typename T, typename U, typename V>
  bool deleteEntities(T& entities, U& modified, V& expunged, bool debugLog);

  void observe(ManagerEventType event, ConditionCallback functionHandle, void* callData);
  void observe(ManagerEventType event, OneToOneCallback functionHandle, void* callData);
  void observe(ManagerEventType event, OneToManyCallback functionHandle, void* callData);
  void observe(OperatorEventType event, BareOperatorCallback functionHandle, void* callData);
  void unobserve(ManagerEventType event, ConditionCallback functionHandle, void* callData);
  void unobserve(ManagerEventType event, OneToOneCallback functionHandle, void* callData);
  void unobserve(ManagerEventType event, OneToManyCallback functionHandle, void* callData);
  void unobserve(OperatorEventType event, BareOperatorCallback functionHandle, void* callData);
  void trigger(ManagerEventType event, const smtk::model::EntityRef& src);
  void trigger(ManagerEventType event, const smtk::model::EntityRef& src, const smtk::model::EntityRef& related);
  void trigger(ManagerEventType event, const smtk::model::EntityRef& src, const smtk::model::EntityRefArray& related);
  void trigger(OperatorEventType event, const smtk::model::Operator& src);

  smtk::io::Logger& log() { return this->m_log; }

protected:
  friend class smtk::attribute::System;

  void assignDefaultNamesWithOwner(
    const UUIDWithEntity& irec,
    const smtk::common::UUID& owner,
    const std::string& ownersName,
    std::set<smtk::common::UUID>& remaining,
    bool nokids);
  std::string assignDefaultName(const smtk::common::UUID& uid, BitFlags entityFlags);
  IntegerList& entityCounts(const smtk::common::UUID& modelId, BitFlags entityFlags);
  void prepareForEntity(std::pair<smtk::common::UUID,Entity>& entry);

  // Below are all the different things that can be mapped to a UUID:
  smtk::shared_ptr<UUIDsToEntities> m_topology;
  smtk::shared_ptr<UUIDsToFloatData> m_floatData;
  smtk::shared_ptr<UUIDsToStringData> m_stringData;
  smtk::shared_ptr<UUIDsToIntegerData> m_integerData;
  smtk::shared_ptr<UUIDsToArrangements> m_arrangements;
  smtk::shared_ptr<UUIDsToTessellations> m_tessellations;
  smtk::shared_ptr<UUIDsToTessellations> m_analysisMesh;
  smtk::shared_ptr<smtk::mesh::Manager> m_meshes;
  smtk::shared_ptr<UUIDsToAttributeAssignments> m_attributeAssignments;
  smtk::shared_ptr<UUIDsToSessions> m_sessions;

  smtk::shared_ptr<Session> m_defaultSession;
  smtk::common::UUIDGenerator m_uuidGenerator;

  IntegerList m_globalCounters; // first entry is session counter, second is model counter

  std::set<ConditionTrigger> m_conditionTriggers;
  std::set<OneToOneTrigger> m_oneToOneTriggers;
  std::set<OneToManyTrigger> m_oneToManyTriggers;
  std::set<BareOperatorTrigger> m_operatorTriggers;

  smtk::io::Logger m_log;
};

template<typename Collection>
Collection Manager::findEntitiesByPropertyAs(const std::string& pname, Integer pval)
{
  Collection collection;
  UUIDWithIntegerProperties pit;
  for (pit = this->m_integerData->begin(); pit != this->m_integerData->end(); ++pit)
    {
    PropertyNameWithIntegers it;
    for (it = pit->second.begin(); it != pit->second.end(); ++it)
      {
      if (it->first == pname && it->second.size() == 1 && it->second[0] == pval)
        {
        typename Collection::value_type entry(shared_from_this(), pit->first);
        if (entry.isValid())
          collection.insert(collection.end(), entry);
        }
      }
    }
  return collection;
}

template<typename Collection>
Collection Manager::findEntitiesByPropertyAs(const std::string& pname, const IntegerList& pval)
{
  Collection collection;
  UUIDWithIntegerProperties pit;
  for (pit = this->m_integerData->begin(); pit != this->m_integerData->end(); ++pit)
    {
    PropertyNameWithIntegers it;
    for (it = pit->second.begin(); it != pit->second.end(); ++it)
      {
      if (it->first == pname && it->second == pval)
        {
        typename Collection::value_type entry(shared_from_this(), pit->first);
        if (entry.isValid())
          collection.insert(collection.end(), entry);
        }
      }
    }
  return collection;
}

template<typename Collection>
Collection Manager::findEntitiesByPropertyAs(const std::string& pname, Float pval)
{
  Collection collection;
  UUIDWithFloatProperties pit;
  for (pit = this->m_floatData->begin(); pit != this->m_floatData->end(); ++pit)
    {
    PropertyNameWithFloats it;
    for (it = pit->second.begin(); it != pit->second.end(); ++it)
      {
      if (it->first == pname && it->second.size() == 1 && it->second[0] == pval)
        {
        typename Collection::value_type entry(shared_from_this(), pit->first);
        if (entry.isValid())
          collection.insert(collection.end(), entry);
        }
      }
    }
  return collection;
}

template<typename Collection>
Collection Manager::findEntitiesByPropertyAs(const std::string& pname, const FloatList& pval)
{
  Collection collection;
  UUIDWithFloatProperties pit;
  for (pit = this->m_floatData->begin(); pit != this->m_floatData->end(); ++pit)
    {
    PropertyNameWithFloats it;
    for (it = pit->second.begin(); it != pit->second.end(); ++it)
      {
      if (it->first == pname && it->second == pval)
        {
        typename Collection::value_type entry(shared_from_this(), pit->first);
        if (entry.isValid())
          collection.insert(collection.end(), entry);
        }
      }
    }
  return collection;
}

template<typename Collection>
Collection Manager::findEntitiesByPropertyAs(const std::string& pname, const std::string& pval)
{
  Collection collection;
  UUIDWithStringProperties pit;
  for (pit = this->m_stringData->begin(); pit != this->m_stringData->end(); ++pit)
    {
    PropertyNameWithStrings it;
    for (it = pit->second.begin(); it != pit->second.end(); ++it)
      {
      if (it->first == pname && it->second.size() == 1 && it->second[0] == pval)
        {
        typename Collection::value_type entry(shared_from_this(), pit->first);
        if (entry.isValid())
          collection.insert(collection.end(), entry);
        }
      }
    }
  return collection;
}

template<typename Collection>
Collection Manager::findEntitiesByPropertyAs(const std::string& pname, const StringList& pval)
{
  Collection collection;
  UUIDWithStringProperties pit;
  for (pit = this->m_stringData->begin(); pit != this->m_stringData->end(); ++pit)
    {
    PropertyNameWithStrings it;
    for (it = pit->second.begin(); it != pit->second.end(); ++it)
      {
      if (it->first == pname && it->second == pval)
        {
        typename Collection::value_type entry(shared_from_this(), pit->first);
        if (entry.isValid())
          collection.insert(collection.end(), entry);
        }
      }
    }
  return collection;
}

template<typename Collection>
Collection Manager::entitiesMatchingFlagsAs(BitFlags mask, bool exactMatch)
{
  smtk::common::UUIDs matches = this->entitiesMatchingFlags(mask, exactMatch);
  Collection collection;
  for (smtk::common::UUIDs::iterator it = matches.begin(); it != matches.end(); ++it)
    {
    typename Collection::value_type entry(shared_from_this(), *it);
    if (entry.isValid())
      collection.insert(collection.end(), entry);
    }
  return collection;
}

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Manager_h
