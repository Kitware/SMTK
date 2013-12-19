#include "smtk/model/FaceUse.h"

#include "smtk/model/Edge.h"
#include "smtk/model/Volume.h"
#include "smtk/model/Loop.h"

namespace smtk {
  namespace model {

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


  } // namespace model
} // namespace smtk
