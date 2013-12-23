#ifndef __smtk_model_ModelEntity_h
#define __smtk_model_ModelEntity_h

#include "smtk/model/Cursor.h"

namespace smtk {
  namespace model {

class CellEntity;
class GroupEntity;
class ModelEntity;
typedef std::vector<CellEntity> CellEntities;
typedef std::vector<GroupEntity> GroupEntities;
typedef std::vector<ModelEntity> ModelEntities;

/**\brief A cursor subclass that provides methods specific to models.
  *
  */
class SMTKCORE_EXPORT ModelEntity : public Cursor
{
public:
  SMTK_CURSOR_CLASS(ModelEntity,Cursor,isModelEntity);

  Cursor parent() const;

  CellEntities cells() const;
  GroupEntities groups() const;
  ModelEntities models() const;
};

typedef std::vector<ModelEntity> ModelEntities;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ModelEntity_h
