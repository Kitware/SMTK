#ifndef __smtk_model_FaceUse_h
#define __smtk_model_FaceUse_h

#include "smtk/model/UseEntity.h"

#include <vector>

namespace smtk {
  namespace model {

class Edge;
class EdgeUse;
class Face;
class Loop;
class FaceUse;
class Volume;
typedef std::vector<Loop> Loops;
typedef std::vector<Edge> Edges;
typedef std::vector<EdgeUse> EdgeUses;
typedef std::vector<FaceUse> FaceUses;

/**\brief A cursor subclass that provides methods specific to 0-d vertex cells.
  *
  */
class SMTKCORE_EXPORT FaceUse : public UseEntity
{
public:
  SMTK_CURSOR_CLASS(FaceUse,UseEntity,isFaceUse);

  Volume volume() const; // The volume bounded by this face use (if any)
  Edges edges() const; // ordered list of vertices in the sense of this edge use
  EdgeUse ccwUse() const; // the next edge use around the edge
  EdgeUse cwUse() const; // the previous edge use around the edge
  Face face() const; // the (parent) underlying face of this use
  Loops loops() const; // The toplevel boundary loops for this face (hole-loops not included)
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_FaceUse_h
