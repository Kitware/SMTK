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

#include "smtk/model/BRepModel.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/Arrangement.h"
#include "smtk/model/AttributeAssignments.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Events.h"
#include "smtk/model/Tessellation.h"

#include <algorithm>
#include <set>
#include <map>
#include <vector>

#include <sstream>

namespace smtk {
  namespace model {

/**\brief Store information about solid models.
  *
  * This adds information about arrangements and tessellations
  * of entities to its BRepModel base class.
  */
class SMTKCORE_EXPORT Manager : public BRepModel
{
public:
  typedef UUIDsToTessellations::iterator tess_iter_type;

  smtkTypeMacro(Manager);
  smtkCreateMacro(Manager);
  smtkSharedFromThisMacro(BRepModel);
  Manager();
  Manager(
    shared_ptr<UUIDsToEntities> topology,
    shared_ptr<UUIDsToArrangements> arrangements,
    shared_ptr<UUIDsToTessellations> tess,
    shared_ptr<UUIDsToAttributeAssignments> attribs);
  virtual ~Manager();

  UUIDsToArrangements& arrangements();
  const UUIDsToArrangements& arrangements() const;

  UUIDsToTessellations& tessellations();
  const UUIDsToTessellations& tessellations() const;

  UUIDsToAttributeAssignments& attributeAssignments();
  const UUIDsToAttributeAssignments& attributeAssignments() const;

  smtk::attribute::System* attributeSystem() const;

  CursorArray findEntitiesByProperty(const std::string& pname, Integer pval);
  CursorArray findEntitiesByProperty(const std::string& pname, Float pval);
  CursorArray findEntitiesByProperty(const std::string& pname, const std::string& pval);
  CursorArray findEntitiesByProperty(const std::string& pname, const IntegerList& pval);
  CursorArray findEntitiesByProperty(const std::string& pname, const FloatList& pval);
  CursorArray findEntitiesByProperty(const std::string& pname, const StringList& pval);
  CursorArray findEntitiesOfType(BitFlags flags, bool exactMatch = true);

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

  using BRepModel::erase;
  virtual bool erase(const Cursor& cursor);
  virtual bool eraseModel(const ModelEntity& cursor);

  tess_iter_type setTessellation(const smtk::common::UUID& cellId, const Tessellation& geom);

  int arrangeEntity(const smtk::common::UUID& entityId, ArrangementKind, const Arrangement& arr, int index = -1);
  int unarrangeEntity(const smtk::common::UUID& entityId, ArrangementKind, int index, bool removeIfLast = false);

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

  int findCellHasUseWithSense(const smtk::common::UUID& cellId, int sense) const;
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
  bool attachAttribute(const smtk::common::UUID&  attribId, const smtk::common::UUID& toEntity);
  bool detachAttribute(const smtk::common::UUID&  attribId, const smtk::common::UUID& fromEntity, bool reverse = true);

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

  GroupEntity insertGroup(
    const smtk::common::UUID& uid,
    int extraFlags = 0,
    const std::string& name = std::string());
  GroupEntity addGroup(int extraFlags = 0, const std::string& name = std::string());

  ModelEntity insertModel(
    const smtk::common::UUID& uid,
    int parametricDim = 3,
    int embeddingDim = 3,
    const std::string& name = std::string());
  ModelEntity addModel(
    int parametricDim = 3, int embeddingDim = 3, const std::string& name = std::string());

  InstanceEntity addInstance();
  InstanceEntity addInstance(const Cursor& instanceOf);

  void observe(ManagerEventType event, ConditionCallback functionHandle, void* callData);
  void observe(ManagerEventType event, OneToOneCallback functionHandle, void* callData);
  void observe(ManagerEventType event, OneToManyCallback functionHandle, void* callData);
  void unobserve(ManagerEventType event, ConditionCallback functionHandle, void* callData);
  void unobserve(ManagerEventType event, OneToOneCallback functionHandle, void* callData);
  void unobserve(ManagerEventType event, OneToManyCallback functionHandle, void* callData);
  void trigger(ManagerEventType event, const smtk::model::Cursor& src);
  void trigger(ManagerEventType event, const smtk::model::Cursor& src, const smtk::model::Cursor& related);
  void trigger(ManagerEventType event, const smtk::model::Cursor& src, const smtk::model::CursorArray& related);

protected:
  friend class smtk::attribute::System;
  bool setAttributeSystem(smtk::attribute::System* sys, bool reverse = true);

  std::set<ConditionTrigger> m_conditionTriggers;
  std::set<OneToOneTrigger> m_oneToOneTriggers;
  std::set<OneToManyTrigger> m_oneToManyTriggers;
  shared_ptr<UUIDsToArrangements> m_arrangements;
  shared_ptr<UUIDsToTessellations> m_tessellations;
  shared_ptr<UUIDsToAttributeAssignments> m_attributeAssignments;
  smtk::attribute::System* m_attributeSystem;
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
