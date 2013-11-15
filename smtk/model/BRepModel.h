#ifndef __smtk_model_BRepModel_h
#define __smtk_model_BRepModel_h

#include "smtk/util/UUID.h"

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/model/Entity.h"
#include "smtk/model/StringData.h"

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

  google::sparse_hash_map<smtk::util::UUID,Entity>& topology();
  const google::sparse_hash_map<smtk::util::UUID,Entity>& topology() const;

  int type(const smtk::util::UUID& ofEntity);
  int dimension(const smtk::util::UUID& ofEntity);

  const Entity* findEntity(const smtk::util::UUID& uid) const;
  Entity* findEntity(const smtk::util::UUID& uid);

  UUIDs bordantEntities(const smtk::util::UUID& ofEntity, int ofDimension = -2);
  UUIDs bordantEntities(const UUIDs& ofEntities, int ofDimension = -2);
  UUIDs boundaryEntities(const smtk::util::UUID& ofEntity, int ofDimension = -2);
  UUIDs boundaryEntities(const UUIDs& ofEntities, int ofDimension = -2);

  UUIDs lowerDimensionalBoundaries(const smtk::util::UUID& ofEntity, int lowerDimension);
  UUIDs higherDimensionalBordants(const smtk::util::UUID& ofEntity, int higherDimension);
  UUIDs adjacentEntities(const smtk::util::UUID& ofEntity, int ofDimension);

  UUIDs entitiesMatchingFlags(unsigned int mask, bool exactMatch = true);
  UUIDs entitiesOfDimension(int dim);

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
  void setDeleteStorage(bool d);


  void setStringProperty(const smtk::util::UUID& entity, const std::string propName, const std::string& propValue)
    {
    smtk::model::StringList tmp;
    tmp.push_back(propValue);
    this->setStringProperty(entity, propName, tmp);
    }

  void setStringProperty(const smtk::util::UUID& entity, const std::string propName, const smtk::model::StringList& propValue)
    {
    (*this->m_stringData)[entity][propName] = propValue;
    }

  smtk::model::StringList const& stringProperty(const smtk::util::UUID& entity, const std::string propName) const
    {
    StringData& strings((*this->m_stringData)[entity]);
    return strings[propName];
    }

  smtk::model::StringList& stringProperty(const smtk::util::UUID& entity, const std::string propName)
    {
    StringData& strings((*this->m_stringData)[entity]);
    return strings[propName];
    }

  bool hasStringProperty(const smtk::util::UUID& entity, const std::string propName) const
    {
    UUIDsToStringData::const_iterator uit = this->m_stringData->find(entity);
    if (uit == this->m_stringData->end())
      {
      return false;
      }
    StringData::const_iterator sit = uit->second.find(propName);
    return sit == uit->second.end() ? false : true;
    }

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

  void addToGroup(const smtk::util::UUID& groupId, const UUIDs& uids);

protected:
  UUIDsToEntities* m_topology;
  UUIDsToStringData* m_stringData;
  bool m_deleteStorage;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_BRepModel_h
