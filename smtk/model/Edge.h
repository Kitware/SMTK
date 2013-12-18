#ifndef __smtk_model_Edge_h
#define __smtk_model_Edge_h

#include "smtk/model/CellEntity.h"

//#include "smtk/util/Eigen.h" // For Vector3d

#include <vector>

namespace smtk {
  namespace model {

class Vertex;
typedef std::vector<Vertex> Vertices;

/**\brief A cursor subclass that provides methods specific to 0-d vertex cells.
  *
  */
class SMTKCORE_EXPORT Edge : public CellEntity
{
public:
  SMTK_CURSOR_CLASS(Edge,CellEntity,isEdge);

  Vertices vertices() const;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Edge_h
