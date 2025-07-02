//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_Resource_h
#define smtk_model_Resource_h
/*!\file */

#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SharedPtr.h"
#include "smtk/SystemConfig.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/AttributeAssignments.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Events.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/Session.h"
#include "smtk/model/SessionRef.h"
#include "smtk/model/StringData.h"
#include "smtk/model/Tessellation.h"

#include "smtk/geometry/Resource.h"

#include "smtk/resource/DerivedFrom.h"

#include "smtk/common/UUID.h"

#include "smtk/io/Logger.h"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

#include <sstream>

/**\brief The name of the model resource used to filter resources of this type.
  *
  * \sa  SMTK_MODEL_RESOURCE_NAME
  */
#define SMTK_MODEL_RESOURCE_NAME "model"

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

/**\brief The name of a float property used to store the bounding box of a model entity or an auxiliary geometry
  *
  * \sa  SMTK_BOUNDING_BOX_PROP
  */
#define SMTK_BOUNDING_BOX_PROP "_boundingBox"

namespace smtk
{
namespace model
{

/// Store information mapping IDs to Entity records. This is the primary storage for SMTK models.
typedef std::map<smtk::common::UUID, EntityPtr> UUIDsToEntities;

/// An abbreviation for an iterator into primary model storage.
typedef UUIDsToEntities::iterator UUIDWithEntityPtr;
typedef UUIDsToEntities::const_iterator UUIDWithConstEntityPtr;
/**\brief Store information about solid models.
  *
  */
class SMTKCORE_EXPORT Resource
  : public smtk::resource::DerivedFrom<Resource, smtk::geometry::Resource>
{
public:
  typedef UUIDsToEntities storage_type;
  typedef storage_type::iterator iter_type;
  typedef UUIDsToTessellations::iterator tess_iter_type;

  smtkTypeMacro(smtk::model::Resource);
  smtkSharedPtrCreateMacro(smtk::resource::PersistentObject);

  // typedef referring to the parent resource.
  typedef smtk::geometry::Resource ParentResource;

  // Associations to other resources and components are managed internally using
  // smtk::resource::Links.
  static constexpr smtk::resource::Links::RoleType AssociationRole = -1;

  // A model resource may be linked to a mesh collection that represents its
  // tessellation. It doesn't have to, though, if the model's tessellation is
  // represented internally.
  static constexpr smtk::resource::Links::RoleType TessellationRole = -2;

  Resource(smtk::resource::ManagerPtr = smtk::resource::ManagerPtr());
  Resource(
    const smtk::common::UUID& uid,
    smtk::resource::ManagerPtr = smtk::resource::ManagerPtr());
  Resource(
    shared_ptr<UUIDsToEntities> topology,
    shared_ptr<UUIDsToTessellations> tess,
    shared_ptr<UUIDsToTessellations> analysismesh,
    shared_ptr<UUIDsToAttributeAssignments> attribs,
    const smtk::common::UUID& uid,
    smtk::resource::ManagerPtr = smtk::resource::ManagerPtr());
  Resource(Resource&& rhs) = default;
  ~Resource() override;

  UUIDsToEntities& topology();
  const UUIDsToEntities& topology() const;

  UUIDsToTessellations& tessellations();
  const UUIDsToTessellations& tessellations() const;

  UUIDsToTessellations& analysisMesh();
  const UUIDsToTessellations& analysisMesh() const;

  /// Remove all entities and properties from this object. Does not change id or emit signals.
  void clear();

  const UUIDsToAttributeAssignments& attributeAssignments() const;

  BitFlags type(const smtk::common::UUID& ofEntity) const;
  int dimension(const smtk::common::UUID& ofEntity) const;

  using smtk::resource::Resource::name;
  std::string name(const smtk::common::UUID& ofEntity) const;

  EntityPtr findEntity(const smtk::common::UUID& uid, bool trySessions = true) const;

  smtk::resource::ComponentPtr find(const smtk::common::UUID& uid) const override;
  std::function<bool(const smtk::resource::Component&)> queryOperation(
    const std::string&) const override;
  void visit(smtk::resource::Component::Visitor&) const override;

  virtual SessionInfoBits erase(
    const smtk::common::UUID& uid,
    SessionInfoBits flags = smtk::model::SESSION_EVERYTHING);
  virtual SessionInfoBits erase(
    const EntityRef& entityref,
    SessionInfoBits flags = smtk::model::SESSION_EVERYTHING);
  virtual SessionInfoBits erase(
    const EntityPtr& entityrec,
    SessionInfoBits flags = smtk::model::SESSION_EVERYTHING);
  virtual SessionInfoBits eraseModel(
    const Model& entityref,
    SessionInfoBits flags = smtk::model::SESSION_EVERYTHING);
  SessionInfoBits hardErase(
    const EntityRef& eref,
    SessionInfoBits flags = smtk::model::SESSION_EVERYTHING);

  smtk::common::UUIDs bordantEntities(const smtk::common::UUID& ofEntity, int ofDimension = -2)
    const;
  smtk::common::UUIDs bordantEntities(const smtk::common::UUIDs& ofEntities, int ofDimension = -2)
    const;
  smtk::common::UUIDs boundaryEntities(const smtk::common::UUID& ofEntity, int ofDimension = -2)
    const;
  smtk::common::UUIDs boundaryEntities(const smtk::common::UUIDs& ofEntities, int ofDimension = -2)
    const;

  smtk::common::UUIDs lowerDimensionalBoundaries(
    const smtk::common::UUID& ofEntity,
    int lowerDimension);
  smtk::common::UUIDs higherDimensionalBordants(
    const smtk::common::UUID& ofEntity,
    int higherDimension);
  smtk::common::UUIDs adjacentEntities(const smtk::common::UUID& ofEntity, int ofDimension);

  smtk::common::UUIDs entitiesMatchingFlags(BitFlags mask, bool exactMatch = true);
  smtk::common::UUIDs entitiesOfDimension(int dim);

  smtk::common::UUID unusedUUID();
  iter_type insertEntityOfTypeAndDimension(BitFlags entityFlags, int dim);
  iter_type insertEntity(EntityPtr cell);
  iter_type
  setEntityOfTypeAndDimension(const smtk::common::UUID& uid, BitFlags entityFlags, int dim);
  iter_type setEntity(EntityPtr cell);

  smtk::common::UUID addEntityOfTypeAndDimension(BitFlags entityFlags, int dim);
  smtk::common::UUID addEntity(EntityPtr cell);
  smtk::common::UUID
  addEntityOfTypeAndDimensionWithUUID(const smtk::common::UUID& uid, BitFlags entityFlags, int dim);
  smtk::common::UUID addEntityWithUUID(const smtk::common::UUID& uid, EntityPtr cell);

  iter_type insertCellOfDimension(int dim);
  iter_type setCellOfDimension(const smtk::common::UUID& uid, int dim);
  smtk::common::UUID addCellOfDimension(int dim);
  smtk::common::UUID addCellOfDimensionWithUUID(const smtk::common::UUID& uid, int dim);

  void insertEntityReferences(const UUIDWithEntityPtr& c);
  bool elideOneEntityReference(const UUIDWithEntityPtr& c, const smtk::common::UUID& r);
  void elideEntityReferences(const UUIDWithEntityPtr& c);
  void removeEntityReferences(const UUIDWithEntityPtr& c);

  virtual void addToGroup(const smtk::common::UUID& groupId, const smtk::common::UUIDs& uids);

  void setFloatProperty(
    const smtk::common::UUID& entity,
    const std::string& propName,
    smtk::model::Float propValue);
  void setFloatProperty(
    const smtk::common::UUID& entity,
    const std::string& propName,
    const smtk::model::FloatList& propValue);
  smtk::model::FloatList const& floatProperty(
    const smtk::common::UUID& entity,
    const std::string& propName) const;
  smtk::model::FloatList& floatProperty(
    const smtk::common::UUID& entity,
    const std::string& propName);
  bool hasFloatProperty(const smtk::common::UUID& entity, const std::string& propName) const;
  bool removeFloatProperty(const smtk::common::UUID& entity, const std::string& propName);

  void setStringProperty(
    const smtk::common::UUID& entity,
    const std::string& propName,
    const smtk::model::String& propValue);
  void setStringProperty(
    const smtk::common::UUID& entity,
    const std::string& propName,
    const smtk::model::StringList& propValue);
  smtk::model::StringList const& stringProperty(
    const smtk::common::UUID& entity,
    const std::string& propName) const;
  smtk::model::StringList& stringProperty(
    const smtk::common::UUID& entity,
    const std::string& propName);
  bool hasStringProperty(const smtk::common::UUID& entity, const std::string& propName) const;
  bool removeStringProperty(const smtk::common::UUID& entity, const std::string& propName);

  void setIntegerProperty(
    const smtk::common::UUID& entity,
    const std::string& propName,
    smtk::model::Integer propValue);
  void setIntegerProperty(
    const smtk::common::UUID& entity,
    const std::string& propName,
    const smtk::model::IntegerList& propValue);
  smtk::model::IntegerList const& integerProperty(
    const smtk::common::UUID& entity,
    const std::string& propName) const;
  smtk::model::IntegerList& integerProperty(
    const smtk::common::UUID& entity,
    const std::string& propName);
  bool hasIntegerProperty(const smtk::common::UUID& entity, const std::string& propName) const;
  bool removeIntegerProperty(const smtk::common::UUID& entity, const std::string& propName);

  smtk::common::UUID modelOwningEntity(const smtk::common::UUID& uid) const;
  smtk::common::UUID sessionOwningEntity(const smtk::common::UUID& uid) const;

  void assignDefaultNames();
  void assignDefaultNamesToModelChildren(const smtk::common::UUID& modelId);
  std::string assignDefaultName(const smtk::common::UUID& uid);
  std::string assignDefaultNameIfMissing(const smtk::common::UUID& uid);
  static std::string shortUUIDName(const smtk::common::UUID& uid, BitFlags entityFlags);

  bool closeSession(const SessionRef& sess);

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

  template<typename Collection, typename Type>
  Collection findEntitiesByTypeAndPropertyAs(const std::string& pname, const Type& pval);
  template<typename Collection, typename Type>
  Collection findEntitiesByTypeAndPropertyAs(
    const std::string& pname,
    const std::vector<Type>& pval);

  template<typename Collection>
  Collection findEntitiesByPropertyAs(const std::string& pname, long pval)
  {
    return findEntitiesByTypeAndPropertyAs<Collection, long>(pname, pval);
  }
  template<typename Collection>
  Collection findEntitiesByPropertyAs(const std::string& pname, const std::vector<long>& pval)
  {
    return findEntitiesByTypeAndPropertyAs<Collection, long>(pname, pval);
  }
  template<typename Collection>
  Collection findEntitiesByPropertyAs(const std::string& pname, Float pval)
  {
    return findEntitiesByTypeAndPropertyAs<Collection, double>(pname, pval);
  }
  template<typename Collection>
  Collection findEntitiesByPropertyAs(const std::string& pname, const std::vector<double>& pval)
  {
    return findEntitiesByTypeAndPropertyAs<Collection, double>(pname, pval);
  }
  template<typename Collection>
  Collection findEntitiesByPropertyAs(const std::string& pname, const std::string& pval)
  {
    return findEntitiesByTypeAndPropertyAs<Collection, std::string>(pname, pval);
  }
  template<typename Collection>
  Collection findEntitiesByPropertyAs(
    const std::string& pname,
    const std::vector<std::string>& pval)
  {
    return findEntitiesByTypeAndPropertyAs<Collection, std::string>(pname, pval);
  }
  template<typename Collection>
  Collection entitiesMatchingFlagsAs(BitFlags flags, bool exactMatch = true);

  tess_iter_type setTessellation(
    const smtk::common::UUID& cellId,
    const Tessellation& geom,
    int analysis = 0,
    int* gen = nullptr);

  tess_iter_type setTessellationAndBoundingBox(
    const smtk::common::UUID& cellId,
    const Tessellation& geom,
    int analysis = 0,
    int* gen = nullptr);
  // if BBox is provided, just replace corrds with BBox else we would loop through coords
  // BBox format: [xmin, xmax, ymin, ymax, zmin, zmax]
  bool setBoundingBox(
    const smtk::common::UUID& cellId,
    const std::vector<double>& coords,
    int providedBBox = 0);
  bool removeTessellation(const smtk::common::UUID& cellId, bool removeGen = false);

  int arrangeEntity(
    const smtk::common::UUID& entityId,
    ArrangementKind,
    const Arrangement& arr,
    int index = -1);
  int unarrangeEntity(
    const smtk::common::UUID& entityId,
    ArrangementKind,
    int index,
    bool removeIfLast = false);
  bool clearArrangements(const smtk::common::UUID& entityId);

  const Arrangements* hasArrangementsOfKindForEntity(
    const smtk::common::UUID& cellId,
    ArrangementKind) const;
  Arrangements* hasArrangementsOfKindForEntity(const smtk::common::UUID& cellId, ArrangementKind);

  Arrangements& arrangementsOfKindForEntity(const smtk::common::UUID& cellId, ArrangementKind);

  const Arrangement*
  findArrangement(const smtk::common::UUID& entityId, ArrangementKind kind, int index) const;
  Arrangement* findArrangement(const smtk::common::UUID& entityId, ArrangementKind kind, int index);
  int findArrangementInvolvingEntity(
    const smtk::common::UUID& entityId,
    ArrangementKind kind,
    const smtk::common::UUID& involved) const;
  bool findDualArrangements(
    const smtk::common::UUID& entityId,
    ArrangementKind kind,
    int index,
    ArrangementReferences& duals) const;
  bool addDualArrangement(
    const smtk::common::UUID& parent,
    const smtk::common::UUID& child,
    ArrangementKind kind,
    int sense,
    Orientation orientation);

  int findCellHasUseWithSense(
    const smtk::common::UUID& cellId,
    const smtk::common::UUID& use,
    int sense) const;
  std::set<int> findCellHasUsesWithOrientation(const smtk::common::UUID& cellId, Orientation orient)
    const;

  smtk::common::UUID
  cellHasUseOfSenseAndOrientation(const smtk::common::UUID& cell, int sense, Orientation o) const;
  smtk::common::UUID findCreateOrReplaceCellUseOfSenseAndOrientation(
    const smtk::common::UUID& cell,
    int sense,
    Orientation o,
    const smtk::common::UUID& replacement = smtk::common::UUID::null());

  smtk::common::UUIDs useOrShellIncludesShells(const smtk::common::UUID& cellUseOrShell) const;
  smtk::common::UUID createIncludedShell(const smtk::common::UUID& cellUseOrShell);
  bool findOrAddIncludedShell(
    const smtk::common::UUID& parentUseOrShell,
    const smtk::common::UUID& shellToInclude);

  //bool shellHasUse(const smtk::common::UUID& shell, const smtk::common::UUID& use) const;
  //smtk::common::UUIDs shellHasUses(const smtk::common::UUID& shell) const;
  bool findOrAddUseToShell(const smtk::common::UUID& shell, const smtk::common::UUID& use);

  bool findOrAddInclusionToCellOrModel(
    const smtk::common::UUID& cell,
    const smtk::common::UUID& inclusion);

  bool findOrAddEntityToGroup(const smtk::common::UUID& grp, const smtk::common::UUID& ent);

  // Return a set of resources associated to this model resource.
  smtk::resource::ResourceSet associations() const;

  // Add a resource to the set of associated resources, and return true if the
  // association is successful.
  bool associate(const smtk::resource::ResourcePtr& resource);

  // Remove a resource from the set of associated resources, and return true if
  // the disassociation is successful.
  bool disassociate(const smtk::resource::ResourcePtr& resource);

  bool hasAttribute(const smtk::common::UUID& attribId, const smtk::common::UUID& toEntity);
  bool associateAttribute(
    smtk::attribute::ResourcePtr attResource,
    const smtk::common::UUID& attribId,
    const smtk::common::UUID& toEntity);
  bool disassociateAttribute(
    smtk::attribute::ResourcePtr attResource,
    const smtk::common::UUID& attribId,
    const smtk::common::UUID& fromEntity,
    bool reverse = true);
  bool insertEntityAssociations(
    const EntityRef& modelEntity,
    std::set<smtk::attribute::AttributePtr>& associations);
  std::set<smtk::attribute::AttributePtr> associations(const EntityRef& modelEntity);

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
  Model
  addModel(int parametricDim = 3, int embeddingDim = 3, const std::string& name = std::string());

  Instance addInstance();
  Instance addInstance(const EntityRef& instanceOf);

  template<typename T>
  bool insertModelFaceWithOrientedOuterLoop(
    const smtk::common::UUID& faceId,    // to be created
    const smtk::common::UUID& faceUseId, // to be created
    const smtk::common::UUID& loopId,    // to be created
    const T& orderedEdgesWithOrientation);

  template<typename T>
  bool insertModelFaceOrientedInnerLoop(
    const smtk::common::UUID& loopId,            // to be created
    const smtk::common::UUID& preExistingLoopId, // the outer loop to contain the inner loop
    const T& orderedEdgesWithOrientation);

  template<typename T, typename U, typename V>
  bool deleteEntities(T& entities, U& modified, V& expunged, bool debugLog);

  void observe(ResourceEventType event, ConditionCallback functionHandle, void* callData);
  void observe(ResourceEventType event, OneToOneCallback functionHandle, void* callData);
  void observe(ResourceEventType event, OneToManyCallback functionHandle, void* callData);
  void unobserve(ResourceEventType event, ConditionCallback functionHandle, void* callData);
  void unobserve(ResourceEventType event, OneToOneCallback functionHandle, void* callData);
  void unobserve(ResourceEventType event, OneToManyCallback functionHandle, void* callData);
  void trigger(ResourceEventType event, const smtk::model::EntityRef& src);
  void trigger(
    ResourceEventType event,
    const smtk::model::EntityRef& src,
    const smtk::model::EntityRef& related);
  void trigger(
    ResourceEventType event,
    const smtk::model::EntityRef& src,
    const smtk::model::EntityRefArray& related);

  smtk::io::Logger& log() { return smtk::io::Logger::instance(); }

protected:
  friend class smtk::attribute::Resource;

  void assignDefaultNamesWithOwner(
    const UUIDWithEntityPtr& irec,
    const smtk::common::UUID& owner,
    const std::string& ownersName,
    std::set<smtk::common::UUID>& remaining,
    bool nokids);
  std::string assignDefaultName(const smtk::common::UUID& uid, BitFlags entityFlags);
  IntegerList& entityCounts(const smtk::common::UUID& modelId, BitFlags entityFlags);
  void prepareForEntity(std::pair<smtk::common::UUID, EntityPtr>& entry);

  smtk::common::UUID modelOwningEntityRecursive(
    const smtk::common::UUID& uid,
    std::set<smtk::common::UUID>& visited) const;
  smtk::common::UUID sessionOwningEntityRecursive(
    const smtk::common::UUID& uid,
    std::set<smtk::common::UUID>& visited) const;

  // Below are all the different things that can be mapped to a UUID:
  smtk::shared_ptr<UUIDsToEntities> m_topology;
  smtk::shared_ptr<UUIDsToTessellations> m_tessellations;
  smtk::shared_ptr<UUIDsToTessellations> m_analysisMesh;
  smtk::shared_ptr<UUIDsToAttributeAssignments> m_attributeAssignments;
  smtk::shared_ptr<UUIDsToSessions> m_sessions;
  typedef std::owner_less<smtk::attribute::WeakResourcePtr> ResourceLessThan;
  typedef std::set<smtk::attribute::WeakResourcePtr, ResourceLessThan> WeakResourceSet;
  WeakResourceSet m_attributeResources; // weak references to attribute resources

  smtk::shared_ptr<Session> m_defaultSession;

  IntegerList m_globalCounters; // first entry is session counter, second is model counter

  std::set<ConditionTrigger> m_conditionTriggers;
  std::set<OneToOneTrigger> m_oneToOneTriggers;
  std::set<OneToManyTrigger> m_oneToManyTriggers;
};

template<typename Collection, typename Type>
Collection Resource::findEntitiesByTypeAndPropertyAs(const std::string& pname, const Type& pval)
{
  Collection collection;
  auto& uuidsToValues =
    this->properties().data().at<std::unordered_map<smtk::common::UUID, std::vector<Type>>>(pname);
  for (auto it = uuidsToValues.begin(); it != uuidsToValues.end(); ++it)
  {
    if (it->second.size() == 1 && it->second.at(0) == pval)
    {
      typename Collection::value_type entry(shared_from_this(), it->first);
      if (entry.isValid())
      {
        collection.insert(collection.end(), entry);
      }
    }
  }
  return collection;
}

template<typename Collection, typename Type>
Collection Resource::findEntitiesByTypeAndPropertyAs(
  const std::string& pname,
  const std::vector<Type>& pval)
{
  Collection collection;
  auto& uuidsToValues =
    this->properties().data().at<std::unordered_map<smtk::common::UUID, std::vector<Type>>>(pname);
  for (auto it = uuidsToValues.begin(); it != uuidsToValues.end(); ++it)
  {
    if (it->second == pval)
    {
      typename Collection::value_type entry(shared_from_this(), it->first);
      if (entry.isValid())
      {
        collection.insert(collection.end(), entry);
      }
    }
  }
  return collection;
}

template<typename Collection>
Collection Resource::entitiesMatchingFlagsAs(BitFlags mask, bool exactMatch)
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

} // namespace model
} // namespace smtk

#endif // smtk_model_Resource_h
