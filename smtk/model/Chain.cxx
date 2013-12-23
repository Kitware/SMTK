#include "smtk/model/Chain.h"

#include "smtk/model/CursorArrangementOps.h"
#include "smtk/model/VertexUse.h"
#include "smtk/model/Edge.h"

namespace smtk {
  namespace model {

/**\brief Return the high-dimensional cell whose interior is bounded by this shell.
  *
  */
Edge Chain::edge() const
{
  return this->ShellEntity::parentCell().as<Edge>();
}

/**\brief Return the face-uses composing this shell.
  *
  */
VertexUses Chain::vertexUses() const
{
  return this->ShellEntity::uses<VertexUses>();
}

/**\brief Return the parent shell of this shell (or an invalid shell if unbounded).
  *
  */
Chain Chain::containingChain() const
{
  return this->ShellEntity::containingShellEntity().as<Chain>();
}

/**\brief Return the child shells of this shell, if any.
  *
  */
Chains Chain::containedChains() const
{
  return this->ShellEntity::containedShellEntities<Chains>();
}

  } // namespace model
} // namespace smtk
