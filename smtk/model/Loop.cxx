#include "smtk/model/Loop.h"

#include "smtk/model/CursorArrangementOps.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"

namespace smtk {
  namespace model {

/**\brief Return the high-dimensional cell whose interior is bounded by this shell.
  *
  */
Face Loop::face() const
{
  return this->ShellEntity::boundingCell().as<Face>();
}

/**\brief Return the face-use bounded by this loop.
  *
  */
FaceUse Loop::faceUse() const
{
  return this->ShellEntity::boundingUseEntity().as<FaceUse>();
}

/**\brief Return the edge-uses composing this loop.
  *
  * The edge uses are properly ordered and co-directional
  * with the sense of the loop.
  */
EdgeUses Loop::edgeUses() const
{
  return this->ShellEntity::uses<EdgeUses>();
}

/**\brief Return the parent shell of this shell (or an invalid shell if unbounded).
  *
  */
Loop Loop::containingLoop() const
{
  return this->ShellEntity::containingShellEntity().as<Loop>();
}

/**\brief Return the child shells of this shell, if any.
  *
  */
Loops Loop::containedLoops() const
{
  return this->ShellEntity::containedShellEntities<Loops>();
}

  } // namespace model
} // namespace smtk
