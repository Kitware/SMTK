#ifndef __smtk_model_ShellEntity_h
#define __smtk_model_ShellEntity_h

#include "smtk/model/Cursor.h"

namespace smtk {
  namespace model {

class CellEntity;
class ShellEntity;
typedef std::set<ShellEntity> ShellEntities;
class UseEntity;
typedef std::vector<UseEntity> UseEntities;

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
