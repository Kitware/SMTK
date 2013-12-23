#ifndef __smtk_model_CellEntity_h
#define __smtk_model_CellEntity_h

#include "smtk/model/Cursor.h"
#include "smtk/model/UseEntity.h" // For UseEntities
#include "smtk/model/ShellEntity.h" // For ShellEntities

namespace smtk {
  namespace model {

/**\brief A cursor subclass with methods specific to cell entities.
  *
  */
class SMTKCORE_EXPORT CellEntity : public Cursor
{
public:
  SMTK_CURSOR_CLASS(CellEntity,Cursor,isCellEntity);

  UseEntities uses() const;
  ShellEntities shells() const;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_CellEntity_h
