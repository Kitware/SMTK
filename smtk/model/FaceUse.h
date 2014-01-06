#ifndef __smtk_model_FaceUse_h
#define __smtk_model_FaceUse_h

#include "smtk/model/UseEntity.h"
#include "smtk/model/ArrangementKind.h" // for Orientation

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

  Volume volume() const;
  Shell boundingShell() const;
  Face face() const;
  Loops loops() const;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_FaceUse_h
