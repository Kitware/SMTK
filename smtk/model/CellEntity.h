#ifndef __smtk_model_CellEntity_h
#define __smtk_model_CellEntity_h

#include "smtk/model/Cursor.h"
#include "smtk/model/UseEntity.h" // For UseEntities

namespace smtk {
  namespace model {

class SMTKCORE_EXPORT CellEntity : public Cursor
{
public:
  SMTK_CURSOR_CLASS(CellEntity,Cursor,isCellEntity);

  UseEntities uses() const;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_CellEntity_h
