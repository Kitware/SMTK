//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_BRepModel_h
#define __smtk_model_BRepModel_h

#include "smtk/Options.h" // for SMTK_HASH_STORAGE
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"
#include "smtk/SharedPtr.h"

#include "smtk/model/Session.h"
#include "smtk/model/Entity.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/PropertyType.h"
#include "smtk/model/StringData.h"

#include "smtk/common/UUID.h"
#include "smtk/common/UUIDGenerator.h"

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

/**\brief A solid model whose entities are referenced individually with instances of T and collectively as sets of type S.
  *
  * Entities are stored as instances of C, regardless of their dimension.
  * The class C must provide a dimension() method.
  *
  * This is templated so we can switch to uint32 values if CGM
  * is unable/unwilling to work with UUIDs.
  */
class SMTKCORE_EXPORT BRepModel : smtkEnableSharedPtr(BRepModel)
{
public:
  typedef UUIDsToEntities storage_type;
  typedef storage_type::iterator iter_type;

  smtkTypeMacro(BRepModel);
  smtkCreateMacro(BRepModel);
  BRepModel();
  BRepModel(shared_ptr<storage_type> topology);
  virtual ~BRepModel();

  UUIDsToEntities& topology();
  const UUIDsToEntities& topology() const;

  BitFlags type(const smtk::common::UUID& ofEntity) const;
  int dimension(const smtk::common::UUID& ofEntity) const;
  std::string name(const smtk::common::UUID& ofEntity) const;

  const Entity* findEntity(const smtk::common::UUID& uid, bool trySessions = true) const;
  Entity* findEntity(const smtk::common::UUID& uid, bool trySessions = true);

  virtual bool erase(const smtk::common::UUID& uid);

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
  SessionPtr sessionForModel(const smtk::common::UUID& uid) const;
  void setSessionForModel(SessionPtr session, const smtk::common::UUID& uid);

  void assignDefaultNames();
  std::string assignDefaultName(const smtk::common::UUID& uid);
  static std::string shortUUIDName(const smtk::common::UUID& uid, BitFlags entityFlags);

  static StringList sessionNames();
  static StringData sessionFileTypes(const std::string& bname, const std::string& engine = std::string());
  static SessionPtr createSessionOfType(const std::string& bname);
  SessionPtr createAndRegisterSession(
    const std::string& bname,
    const smtk::common::UUID& sessionSessionId = smtk::common::UUID::null());

  bool registerSession(SessionPtr session);
  bool unregisterSession(SessionPtr session);
  SessionPtr findSession(const smtk::common::UUID& sessionId) const;
  smtk::common::UUIDs sessionSessions() const;
  smtk::common::UUIDs modelsOfSession(const smtk::common::UUID& sessionId) const;

protected:
  shared_ptr<UUIDsToEntities> m_topology;
  smtk::shared_ptr<UUIDsToFloatData> m_floatData;
  smtk::shared_ptr<UUIDsToStringData> m_stringData;
  smtk::shared_ptr<UUIDsToIntegerData> m_integerData;
  UUIDsToSessions m_modelSessions;
  smtk::shared_ptr<Session> m_defaultSession;
  UUIDsToSessions m_sessions;
  smtk::common::UUIDGenerator m_uuidGenerator;
  IntegerList m_globalCounters; // first entry is session counter, second is model counter

  void assignDefaultNamesWithOwner(
    const UUIDWithEntity& irec,
    const smtk::common::UUID& owner,
    const std::string& ownersName,
    std::set<smtk::common::UUID>& remaining,
    bool nokids);
  std::string assignDefaultName(const smtk::common::UUID& uid, BitFlags entityFlags);
  IntegerList& entityCounts(const smtk::common::UUID& modelId, BitFlags entityFlags);
  void prepareForEntity(std::pair<smtk::common::UUID,Entity>& entry);
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_BRepModel_h
