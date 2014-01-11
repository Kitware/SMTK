#ifndef __smtk_model_CellEntity_h
#define __smtk_model_CellEntity_h

#include "smtk/model/Cursor.h"
#include "smtk/model/UseEntity.h" // For UseEntities
#include "smtk/model/ShellEntity.h" // For ShellEntities
#include "smtk/model/CursorArrangementOps.h" // For appendAllRelations

namespace smtk {
  namespace model {

class CellEntity;
class ModelEntity;
typedef std::vector<CellEntity> CellEntities;

/**\brief A cursor subclass with methods specific to cell entities.
  *
  */
class SMTKCORE_EXPORT CellEntity : public Cursor
{
public:
  SMTK_CURSOR_CLASS(CellEntity,Cursor,isCellEntity);

  ModelEntity model() const;
  ShellEntities shellEntities() const;
  CellEntities boundingCells() const;

  template<typename T> T inclusions() const;
  template<typename T> T uses() const;
};

template<typename T>
T CellEntity::inclusions() const
{
  T result;
  CursorArrangementOps::appendAllRelations(*this, INCLUDES, result);
  return result;
}

template<typename T>
T CellEntity::uses() const
{
  T result;
  CursorArrangementOps::appendAllRelations(*this, HAS_USE, result);
  return result;
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_CellEntity_h
