#ifndef __smtk_model_VertexUse_h
#define __smtk_model_VertexUse_h

#include "smtk/model/UseEntity.h"

#include <vector>

namespace smtk {
  namespace model {

class Edge;
class EdgeUse;
class Vertex;
typedef std::vector<Edge> Edges;
typedef std::vector<EdgeUse> EdgeUses;

/**\brief A cursor subclass that provides methods specific to 0-d vertex cells.
  *
  */
class SMTKCORE_EXPORT VertexUse : public UseEntity
{
public:
  SMTK_CURSOR_CLASS(VertexUse,UseEntity,isVertexUse);

  Vertex vertex() const;
  Edges edges() const;
  EdgeUses edgeUses() const;
};

typedef std::vector<VertexUse> VertexUses;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_VertexUse_h
