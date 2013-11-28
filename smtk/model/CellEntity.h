#ifndef __smtk_model_CellEntity_h
#define __smtk_model_CellEntity_h

#include "smtk/model/Cursor.h"
#include "smtk/model/UseEntity.h" // For UseEntities

namespace smtk {
  namespace model {

class SMTKCORE_EXPORT CellEntity : public Cursor
{
public:
  CellEntity();
  CellEntity(const Cursor&);
  CellEntity(StoragePtr storage, const smtk::util::UUID& uid);

  virtual bool isValid() const
    { return this->Cursor::isValid() && this->isCellEntity(); }

  UseEntities uses() const;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_CellEntity_h
