#ifndef __smtk_model_UseEntity_h
#define __smtk_model_UseEntity_h

#include "smtk/model/Cursor.h"

namespace smtk {
  namespace model {

class CellEntity;

class SMTKCORE_EXPORT UseEntity : public Cursor
{
public:
  UseEntity();
  UseEntity(const Cursor&);
  UseEntity(StoragePtr storage, const smtk::util::UUID& uid);

  virtual bool isValid() const
    { return this->Cursor::isValid() && this->isUseEntity(); }

  CellEntity cell() const;
};

typedef std::set<UseEntity> UseEntities;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_UseEntity_h
