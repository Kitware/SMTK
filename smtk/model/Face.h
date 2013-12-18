#ifndef __smtk_model_Face_h
#define __smtk_model_Face_h

#include "smtk/model/CellEntity.h"

//#include "smtk/util/Eigen.h" // For Vector3d

#include <vector>

namespace smtk {
  namespace model {

class Edge;
class Volume;
typedef std::vector<Edge> Edges;
typedef std::vector<Volume> Volumes;

/**\brief A cursor subclass that provides methods specific to 0-d vertex cells.
  *
  */
class SMTKCORE_EXPORT Face : public CellEntity
{
public:
  SMTK_CURSOR_CLASS(Face,CellEntity,isFace);

  Edges edges() const;
  Volumes volumes() const;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Face_h
