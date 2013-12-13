#ifndef __smtk_model_UseEntity_h
#define __smtk_model_UseEntity_h

#include "smtk/model/Cursor.h"

namespace smtk {
  namespace model {

class CellEntity;

/**\brief A cursor subclass that provides methods specific to entity use records.
  *
  */
class SMTKCORE_EXPORT UseEntity : public Cursor
{
public:
  SMTK_CURSOR_CLASS(UseEntity,Cursor,isUseEntity);

  CellEntity cell() const;
};

typedef std::vector<UseEntity> UseEntities;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_UseEntity_h
