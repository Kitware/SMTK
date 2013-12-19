#ifndef __smtk_model_EdgeUse_h
#define __smtk_model_EdgeUse_h

#include "smtk/model/UseEntity.h"

#include <vector>

namespace smtk {
  namespace model {

class Vertex;
class VertexUse;
class Edge;
class EdgeUse;
class Face;
class Loop;
typedef std::vector<EdgeUse> EdgeUses;
typedef std::vector<Vertex> Vertices;
typedef std::vector<VertexUse> VertexUses;

/**\brief A cursor subclass that provides methods specific to 1-d edge cells.
  *
  */
class SMTKCORE_EXPORT EdgeUse : public UseEntity
{
public:
  SMTK_CURSOR_CLASS(EdgeUse,UseEntity,isEdgeUse);

  VertexUses vertexUses() const; // ordered list of vertex uses for this edge use
  Vertices vertices() const; // ordered list of vertices in the sense of this edge use
  EdgeUse ccwUse() const; // the next edge use around the edge
  EdgeUse cwUse() const; // the previous edge use around the edge
  Edge edge() const; // the (parent) underlying edge of this use
  Loop loop() const;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_EdgeUse_h
