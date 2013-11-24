#ifndef __smtk_model_BRepModel_h
#define __smtk_model_BRepModel_h

#include "smtk/util/UUID.h"

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/model/Entity.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/StringData.h"
#include "smtk/model/IntegerData.h"

#include "sparsehash/sparse_hash_map"

namespace smtk {
  namespace model {

typedef google::sparse_hash_map<smtk::util::UUID,Entity> UUIDsToEntities;
typedef UUIDsToEntities::iterator UUIDWithEntity;

/**\brief A solid model whose entities are referenced individually with instances of T and collectively as sets of type S.
  *
  * Entities are stored as instances of C, regardless of their dimension.
  * The class C must provide a dimension() method.
  *
  * This is templated so we can switch to uint32 values if CGM
  * is unable/unwilling to work with UUIDs.
  */
class SMTKCORE_EXPORT BRepModel
{
public:
  typedef google::sparse_hash_map<smtk::util::UUID,Entity> storage_type;
  typedef storage_type::iterator iter_type;

  BRepModel();
  BRepModel(storage_type* topology, bool shouldDelete);
  ~BRepModel();

  void setDeleteStorage(bool d);

  UUIDsToEntities& topology();
  const UUIDsToEntities& topology() const;

  int type(const smtk::util::UUID& ofEntity);
  int dimension(const smtk::util::UUID& ofEntity);

  const Entity* findEntity(const smtk::util::UUID& uid) const;
  Entity* findEntity(const smtk::util::UUID& uid);

  smtk::util::UUIDs bordantEntities(const smtk::util::UUID& ofEntity, int ofDimension = -2);
  smtk::util::UUIDs bordantEntities(const smtk::util::UUIDs& ofEntities, int ofDimension = -2);
  smtk::util::UUIDs boundaryEntities(const smtk::util::UUID& ofEntity, int ofDimension = -2);
  smtk::util::UUIDs boundaryEntities(const smtk::util::UUIDs& ofEntities, int ofDimension = -2);

  smtk::util::UUIDs lowerDimensionalBoundaries(const smtk::util::UUID& ofEntity, int lowerDimension);
  smtk::util::UUIDs higherDimensionalBordants(const smtk::util::UUID& ofEntity, int higherDimension);
  smtk::util::UUIDs adjacentEntities(const smtk::util::UUID& ofEntity, int ofDimension);

  smtk::util::UUIDs entitiesMatchingFlags(unsigned int mask, bool exactMatch = true);
  smtk::util::UUIDs entitiesOfDimension(int dim);

  iter_type insertEntityOfTypeAndDimension(unsigned int entityFlags, int dim);
  iter_type insertEntity(Entity& cell);
  iter_type setEntityOfTypeAndDimension(const smtk::util::UUID& uid, unsigned int entityFlags, int dim);
  iter_type setEntity(const smtk::util::UUID& uid, Entity& cell);

  smtk::util::UUID addEntityOfTypeAndDimension(unsigned int entityFlags, int dim);
  smtk::util::UUID addEntity(Entity& cell);
  smtk::util::UUID addEntityOfTypeAndDimensionWithUUID(const smtk::util::UUID& uid, unsigned int entityFlags, int dim);
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

  void setStringProperty(const smtk::util::UUID& entity, const std::string& propName, const smtk::model::String& propValue);
  void setStringProperty(const smtk::util::UUID& entity, const std::string& propName, const smtk::model::StringList& propValue);
  smtk::model::StringList const& stringProperty(const smtk::util::UUID& entity, const std::string& propName) const;
  smtk::model::StringList& stringProperty(const smtk::util::UUID& entity, const std::string& propName);
  bool hasStringProperty(const smtk::util::UUID& entity, const std::string& propName) const;
  bool removeStringProperty(const smtk::util::UUID& entity, const std::string& propName);

  void setIntegerProperty(const smtk::util::UUID& entity, const std::string& propName, smtk::model::Integer propValue);
  void setIntegerProperty(const smtk::util::UUID& entity, const std::string& propName, const smtk::model::IntegerList& propValue);
  smtk::model::IntegerList const& integerProperty(const smtk::util::UUID& entity, const std::string& propName) const;
  smtk::model::IntegerList& integerProperty(const smtk::util::UUID& entity, const std::string& propName);
  bool hasIntegerProperty(const smtk::util::UUID& entity, const std::string& propName) const;
  bool removeIntegerProperty(const smtk::util::UUID& entity, const std::string& propName);

  smtk::util::UUID addVertex() { return this->addEntityOfTypeAndDimension(CELL_ENTITY, 0); }
  smtk::util::UUID addEdge() { return this->addEntityOfTypeAndDimension(CELL_ENTITY, 1); }
  smtk::util::UUID addFace() { return this->addEntityOfTypeAndDimension(CELL_ENTITY, 2); }
  smtk::util::UUID addRegion() { return this->addEntityOfTypeAndDimension(CELL_ENTITY, 3); }
  smtk::util::UUID addGroup(int extraFlags = 0, const std::string& name = std::string())
    {
    smtk::util::UUID uid = this->addEntityOfTypeAndDimension(GROUP_ENTITY | extraFlags, -1);
    this->setStringProperty(uid, "name", name);
    return uid;
    }

protected:
  UUIDsToEntities* m_topology;
  UUIDsToFloatData* m_floatData;
  UUIDsToStringData* m_stringData;
  UUIDsToIntegerData* m_integerData;
  bool m_deleteStorage;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_BRepModel_h
