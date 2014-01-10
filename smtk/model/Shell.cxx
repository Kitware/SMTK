#include "smtk/model/Shell.h"

#include "smtk/model/CursorArrangementOps.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Volume.h"

namespace smtk {
  namespace model {

/**\brief Return the high-dimensional cell whose interior is bounded by this shell.
  *
  */
Volume Shell::volume() const
{
  return this->ShellEntity::boundingCell().as<Volume>();
}

/**\brief Return the face-uses composing this shell.
  *
  */
FaceUses Shell::faceUses() const
{
  return this->ShellEntity::uses<FaceUses>();
}

/**\brief Return the parent shell of this shell (or an invalid shell if unbounded).
  *
  */
Shell Shell::containingShell() const
{
  return this->ShellEntity::containingShellEntity().as<Shell>();
}

/**\brief Return the child shells of this shell, if any.
  *
  */
Shells Shell::containedShells() const
{
  return this->ShellEntity::containedShellEntities<Shells>();
}

  } // namespace model
} // namespace smtk
