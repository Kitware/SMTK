#include "smtk/model/FaceUse.h"

#include "smtk/model/Edge.h"
#include "smtk/model/Face.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Shell.h"
#include "smtk/model/Volume.h"

namespace smtk {
  namespace model {

// The volume bounded by this face use (if any)
Volume FaceUse::volume() const
{
  return this->cell().as<Shell>().volume();
}

// ordered list of vertices in the sense of this edge use
Edges FaceUse::edges() const
{
  Edges result;
  Cursors all = this->bordantEntities(/*dim = */ 1);
  for (Cursors::iterator it = all.begin(); it != all.end(); ++it)
    {
    if (it->isEdge())
      result.push_back(*it);
    }
  return result;
}

// the (parent) underlying face of this use
Face FaceUse::face() const
{
  return this->relationFromArrangement(HAS_CELL, 0, 0).as<Face>();
}

// The toplevel boundary loops for this face (hole-loops not included)
Loops FaceUse::loops() const
{
  return this->shellsAs<Loops>();
}

  } // namespace model
} // namespace smtk
