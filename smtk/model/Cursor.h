#ifndef __smtk_model_Cursor_h
#define __smtk_model_Cursor_h

#include "smtk/SMTKCoreExports.h" // For EXPORT macro.
#include "smtk/PublicPointerDefs.h" // For StoragePtr
#include "smtk/model/EntityTypeBits.h" // for BitFlags type

#include "smtk/util/UUID.h"

namespace smtk {
  namespace model {

/**\brief A lightweight cursor pointing to a model entity's storage.
  */
class SMTKCORE_EXPORT Cursor
{
public:
  Cursor();
  Cursor(StoragePtr storage, const smtk::util::UUID& entity);

  bool setStorage(StoragePtr storage);
  StoragePtr storage();

  bool setEntity(const smtk::util::UUID& entity);
  smtk::util::UUID entity();

  int dimension();
  int dimensionBits();
  BitFlags entityFlags();

protected:
  StoragePtr m_storage;
  smtk::util::UUID m_entity;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Cursor_h
