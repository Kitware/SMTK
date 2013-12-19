#include "smtk/model/VertexUse.h"

#include "smtk/model/Edge.h"
#include "smtk/model/Storage.h"
#include "smtk/model/Tessellation.h"

namespace smtk {
  namespace model {

smtk::model::Edges VertexUse::edges() const
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
