#ifndef __smtk_model_Vertex_h
#define __smtk_model_Vertex_h

#include "smtk/model/CellEntity.h"

//#include "smtk/util/Eigen.h" // For Vector3d

#include <vector>

namespace smtk {
  namespace model {

class Edge;
typedef std::vector<Edge> Edges;

/**\brief A cursor subclass that provides methods specific to 0-d vertex cells.
  *
  */
class SMTKCORE_EXPORT Vertex : public CellEntity
{
public:
  SMTK_CURSOR_CLASS(Vertex,CellEntity,isVertex);

  Edges edges() const;

  //smtk::util::Vector3d coordinates() const;
};

typedef std::vector<Vertex> Vertices;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Vertex_h
