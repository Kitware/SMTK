#ifndef __smtk_model_GroupEntity_h
#define __smtk_model_GroupEntity_h

#include "smtk/model/Cursor.h"

namespace smtk {
  namespace model {

class CellEntity;

/**\brief A cursor subclass that provides methods specific to entity use records.
  *
  */
class SMTKCORE_EXPORT GroupEntity : public Cursor
{
public:
  SMTK_CURSOR_CLASS(GroupEntity,Cursor,isGroupEntity);

  CellEntity cell() const;
};

typedef std::vector<GroupEntity> GroupEntities;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_GroupEntity_h
