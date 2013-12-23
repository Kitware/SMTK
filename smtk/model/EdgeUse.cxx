#include "smtk/model/EdgeUse.h"

#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/VertexUse.h"

namespace smtk {
  namespace model {

// ordered list of vertex uses for this edge use
VertexUses EdgeUse::vertexUses() const
{
  VertexUses empty;
  return empty;
}

// ordered list of vertices in the sense of this edge use
Vertices EdgeUse::vertices() const
{
  Vertices empty;
  return empty;
}

// the next edge use around the edge
EdgeUse EdgeUse::ccwUse() const
{
  EdgeUse use;
  return use;
}

// the previous edge use around the edge
EdgeUse EdgeUse::cwUse() const
{
  EdgeUse use;
  return use;
}

// the (parent) underlying edge of this use
Edge EdgeUse::edge() const
{
  return this->relationFromArrangement(HAS_CELL, 0, 0).as<Edge>();
}

/**\brief The loop of the face associated with this edge use (if any) or an invalid entity.
  *
  */
Loop EdgeUse::loop() const
{
  return this->relationFromArrangement(HAS_SHELL, 0, 0).as<Loop>();
}

  } // namespace model
} // namespace smtk
