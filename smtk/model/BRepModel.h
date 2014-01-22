#ifndef __smtk_model_BRepModel_h
#define __smtk_model_BRepModel_h

#include "smtk/util/UUID.h"
#include "smtk/util/UUIDGenerator.h"
#include "smtk/util/SharedFromThis.h"
#include "smtk/util/SystemConfig.h"

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/SharedPtr.h"
#include "smtk/model/Entity.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/StringData.h"
#include "smtk/model/IntegerData.h"

#include "smtk/options.h" // for SMTK_HASH_STORAGE
#ifdef SMTK_HASH_STORAGE
#  include "sparsehash/sparse_hash_map"
#else
#  include <map>
#endif

namespace smtk {
  namespace model {

#ifdef SMTK_HASH_STORAGE
/// Store information mapping IDs to Entity records. This is the primary storage for SMTK models.
typedef google::sparse_hash_map<smtk::util::UUID,Entity> UUIDsToEntities;
#else
/// Store information mapping IDs to Entity records. This is the primary storage for SMTK models.
typedef std::map<smtk::util::UUID,Entity> UUIDsToEntities;
#endif
/// An abbreviation for an iterator into primary model storage.
typedef UUIDsToEntities::iterator UUIDWithEntity;

/// Primitive storage types for model properties
enum PropertyType
{
  FLOAT_PROPERTY,    //!< Property is an array of floating-point numbers
  STRING_PROPERTY,   //!< Property is an array of strings
  INTEGER_PROPERTY,  //!< Property is an array of integers
  INVALID_PROPERTY   //!< Property has no storage.
};

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

  int type(const smtk::util::UUID& ofEntity) const;
  int dimension(const smtk::util::UUID& ofEntity) const;
  std::string name(const smtk::util::UUID& ofEntity) const;

  const Entity* findEntity(const smtk::util::UUID& uid) const;
  Entity* findEntity(const smtk::util::UUID& uid);

  smtk::util::UUIDs bordantEntities(const smtk::util::UUID& ofEntity, int ofDimension = -2);
  smtk::util::UUIDs bordantEntities(const smtk::util::UUIDs& ofEntities, int ofDimension = -2);
  smtk::util::UUIDs boundaryEntities(const smtk::util::UUID& ofEntity, int ofDimension = -2);
  smtk::util::UUIDs boundaryEntities(const smtk::util::UUIDs& ofEntities, int ofDimension = -2);

  smtk::util::UUIDs lowerDimensionalBoundaries(const smtk::util::UUID& ofEntity, int lowerDimension);
  smtk::util::UUIDs higherDimensionalBordants(const smtk::util::UUID& ofEntity, int higherDimension);
  smtk::util::UUIDs adjacentEntities(const smtk::util::UUID& ofEntity, int ofDimension);

  smtk::util::UUIDs entitiesMatchingFlags(BitFlags mask, bool exactMatch = true);
  smtk::util::UUIDs entitiesOfDimension(int dim);

  smtk::util::UUID unusedUUID();
  iter_type insertEntityOfTypeAndDimension(BitFlags entityFlags, int dim);
  iter_type insertEntity(Entity& cell);
  iter_type setEntityOfTypeAndDimension(const smtk::util::UUID& uid, BitFlags entityFlags, int dim);
  iter_type setEntity(const smtk::util::UUID& uid, Entity& cell);

  smtk::util::UUID addEntityOfTypeAndDimension(BitFlags entityFlags, int dim);
  smtk::util::UUID addEntity(Entity& cell);
  smtk::util::UUID addEntityOfTypeAndDimensionWithUUID(const smtk::util::UUID& uid, BitFlags entityFlags, int dim);
  smtk::util::UUID addEntityWithUUID(const smtk::util::UUID& uid, Entity& cell);

  iter_type insertCellOfDimension(int dim);
  iter_type setCellOfDimension(const smtk::util::UUID& uid, int dim);
  smtk::util::UUID addCellOfDimension(int dim);
  smtk::util::UUID addCellOfDimensionWithUUID(const smtk::util::UUID& uid, int dim);

  void insertEntityReferences(const UUIDWithEntity& c);
  void removeEntityReferences(const UUIDWithEntity& c);

  bool removeEntity(const smtk::util::UUID& uid);

  virtual void addToGroup(const smtk::util::UUID& groupId, const smtk::util::UUIDs& uids);

  void setFloatProperty(const smtk::util::UUID& entity, const std::string& propName, smtk::model::Float propValue);
  void setFloatProperty(const smtk::util::UUID& entity, const std::string& propName, const smtk::model::FloatList& propValue);
  smtk::model::FloatList const& floatProperty(const smtk::util::UUID& entity, const std::string& propName) const;
  smtk::model::FloatList& floatProperty(const smtk::util::UUID& entity, const std::string& propName);
  bool hasFloatProperty(const smtk::util::UUID& entity, const std::string& propName) const;
  bool removeFloatProperty(const smtk::util::UUID& entity, const std::string& propName);
  const UUIDWithFloatProperties floatPropertiesForEntity(const smtk::util::UUID& entity) const;
  UUIDWithFloatProperties floatPropertiesForEntity(const smtk::util::UUID& entity);
  UUIDsToFloatData& floatProperties() { return *this->m_floatData; }
  UUIDsToFloatData const& floatProperties() const { return *this->m_floatData; }

  void setStringProperty(const smtk::util::UUID& entity, const std::string& propName, const smtk::model::String& propValue);
  void setStringProperty(const smtk::util::UUID& entity, const std::string& propName, const smtk::model::StringList& propValue);
  smtk::model::StringList const& stringProperty(const smtk::util::UUID& entity, const std::string& propName) const;
  smtk::model::StringList& stringProperty(const smtk::util::UUID& entity, const std::string& propName);
  bool hasStringProperty(const smtk::util::UUID& entity, const std::string& propName) const;
  bool removeStringProperty(const smtk::util::UUID& entity, const std::string& propName);
  const UUIDWithStringProperties stringPropertiesForEntity(const smtk::util::UUID& entity) const;
  UUIDWithStringProperties stringPropertiesForEntity(const smtk::util::UUID& entity);
  UUIDsToStringData& stringProperties() { return *this->m_stringData; }
  UUIDsToStringData const& stringProperties() const { return *this->m_stringData; }

  void setIntegerProperty(const smtk::util::UUID& entity, const std::string& propName, smtk::model::Integer propValue);
  void setIntegerProperty(const smtk::util::UUID& entity, const std::string& propName, const smtk::model::IntegerList& propValue);
  smtk::model::IntegerList const& integerProperty(const smtk::util::UUID& entity, const std::string& propName) const;
  smtk::model::IntegerList& integerProperty(const smtk::util::UUID& entity, const std::string& propName);
  bool hasIntegerProperty(const smtk::util::UUID& entity, const std::string& propName) const;
  bool removeIntegerProperty(const smtk::util::UUID& entity, const std::string& propName);
  const UUIDWithIntegerProperties integerPropertiesForEntity(const smtk::util::UUID& entity) const;
  UUIDWithIntegerProperties integerPropertiesForEntity(const smtk::util::UUID& entity);
  UUIDsToIntegerData& integerProperties() { return *this->m_integerData; }
  UUIDsToIntegerData const& integerProperties() const { return *this->m_integerData; }

  smtk::util::UUID modelOwningEntity(const smtk::util::UUID& uid);

  void assignDefaultNames();
  std::string assignDefaultName(const smtk::util::UUID& uid);
  static std::string shortUUIDName(const smtk::util::UUID& uid, BitFlags entityFlags);

protected:
  shared_ptr<UUIDsToEntities> m_topology;
  smtk::shared_ptr<UUIDsToFloatData> m_floatData;
  smtk::shared_ptr<UUIDsToStringData> m_stringData;
  smtk::shared_ptr<UUIDsToIntegerData> m_integerData;
  smtk::util::UUIDGenerator m_uuidGenerator;
  int m_modelCount;

  std::string assignDefaultName(const smtk::util::UUID& uid, BitFlags entityFlags);
  IntegerList& entityCounts(const smtk::util::UUID& modelId, BitFlags entityFlags);
  void prepareForEntity(std::pair<smtk::util::UUID,Entity>& entry);
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_BRepModel_h
