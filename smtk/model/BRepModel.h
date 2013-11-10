#ifndef __smtk_model_BRepModel_h
#define __smtk_model_BRepModel_h

#include "smtk/util/UUID.h"

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/model/Entity.h"

#include <map>

namespace smtk {
  namespace model {

typedef std::map<smtk::util::UUID,Entity> UUIDsToEntities;
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
  typedef std::map<smtk::util::UUID,Entity> storage_type;
  typedef storage_type::iterator iter_type;

  BRepModel();
  BRepModel(std::map<smtk::util::UUID,Entity>* topology, bool shouldDelete);
  ~BRepModel();

  std::map<smtk::util::UUID,Entity>& topology();
  const std::map<smtk::util::UUID,Entity>& topology() const;

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

  UUIDs entities(int ofDimension);

  iter_type insertEntityOfTypeAndDimension(int entityFlags, int dim);
  iter_type insertEntity(Entity& cell);
  iter_type setEntityOfTypeAndDimension(const smtk::util::UUID& uid, int entityFlags, int dim);
  iter_type setEntity(const smtk::util::UUID& uid, Entity& cell);

  smtk::util::UUID addEntityOfTypeAndDimension(int entityFlags, int dim);
  smtk::util::UUID addEntity(Entity& cell);
  smtk::util::UUID addEntityOfTypeAndDimensionWithUUID(const smtk::util::UUID& uid, int entityFlags, int dim);
  smtk::util::UUID addEntityWithUUID(const smtk::util::UUID& uid, Entity& cell);

  iter_type insertCellOfDimension(int dim);
  iter_type setCellOfDimension(const smtk::util::UUID& uid, int dim);
  smtk::util::UUID addCellOfDimension(int dim);
  smtk::util::UUID addCellOfDimensionWithUUID(const smtk::util::UUID& uid, int dim);

  void insertEntityReferences(const UUIDWithEntity& c);
  void removeEntityReferences(const UUIDWithEntity& c);
  void setDeleteStorage(bool d);

protected:
  UUIDsToEntities* m_topology;
  bool m_deleteStorage;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_BRepModel_h
