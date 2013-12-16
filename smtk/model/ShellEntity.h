#ifndef __smtk_model_ShellEntity_h
#define __smtk_model_ShellEntity_h

#include "smtk/model/Cursor.h"

namespace smtk {
  namespace model {

class CellEntity;
class ShellEntity;
typedef std::vector<ShellEntity> ShellEntities;
class UseEntity;
typedef std::vector<UseEntity> UseEntities;

/**\brief A cursor subclass with methods specific to shell entities.
  *
  * A shell is a collection of oriented cell uses that form a
  * subset of the boundary of a higher-dimensional parent cell.
  * A shell may contain other shells.
  */
class SMTKCORE_EXPORT ShellEntity : public Cursor
{
public:
  SMTK_CURSOR_CLASS(ShellEntity,Cursor,isShellEntity);

  CellEntity parentCell() const;
  UseEntities uses() const;
  ShellEntities containedShells() const;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ShellEntity_h
