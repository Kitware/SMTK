#include "smtk/model/VertexUse.h"

#include "smtk/model/Chain.h"
#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Vertex.h"

namespace smtk {
  namespace model {

Vertex VertexUse::vertex() const
{
  return this->cell().as<Vertex>();
}

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

Chains VertexUse::chains() const
{
  Chains result;
  CursorArrangementOps::appendAllRelations(*this, HAS_SHELL, result);
  return result;
}

  } // namespace model
} // namespace smtk
