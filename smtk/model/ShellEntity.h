#ifndef __smtk_model_ShellEntity_h
#define __smtk_model_ShellEntity_h

#include "smtk/model/Cursor.h"
#include "smtk/model/CursorArrangementOps.h"

namespace smtk {
  namespace model {

class CellEntity;
class ShellEntity;
typedef std::vector<ShellEntity> ShellEntities;
class UseEntity;
typedef std::vector<UseEntity> UseEntities;

/**\brief A cursor subclass with methods specific to shell entities.
  *
  * A shell is a collection of oriented cell-uses that form a
  * subset of the boundary of a higher-dimensional parent cell.
  * A shell may contain other shells.
  */
class SMTKCORE_EXPORT ShellEntity : public Cursor
{
public:
  SMTK_CURSOR_CLASS(ShellEntity,Cursor,isShellEntity);

  CellEntity parentCell() const;
  template<typename T> T uses() const;

  ShellEntity containingShellEntity() const;
  template<typename T> T containedShellEntities() const;
};

/**\brief Return the uses (cells with an orientation, or sense) composing this shell.
  *
  * When the shell is 1-dimensional, these uses will be ordered curve segments
  * that match head-to-tail and describe a closed loop.
  *
  * TODO: Decide how to handle multiple closed loops: should an "empty" shell (that
  * returns uses().empty()) be created with multiple containedShells()? Or should
  * shells be allowed to have siblings?
  */
template<typename T>
T ShellEntity::uses() const
{
  T result;
  CursorArrangementOps::appendAllRelations(*this, HAS_USE, result);
  return result;
}

/**\brief Return all the shell-entities contained inside this one.
  *
  */
template<typename T>
T ShellEntity::containedShellEntities() const
{
  T result;
  CursorArrangementOps::appendAllRelations(*this, INCLUDES, result);
  return result;
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ShellEntity_h
