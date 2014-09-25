#ifndef __smtk_model_Face_h
#define __smtk_model_Face_h

#include "smtk/model/CellEntity.h"

//#include "smtk/common/Eigen.h" // For Vector3d

#include <vector>

namespace smtk {
  namespace model {

class Edge;
class FaceUse;
class Volume;
typedef std::vector<Edge> Edges;
typedef std::vector<FaceUse> FaceUses;
typedef std::vector<Volume> Volumes;

/**\brief A cursor subclass that provides methods specific to 2-d face cells.
  *
  */
class SMTKCORE_EXPORT Face : public CellEntity
{
public:
  SMTK_CURSOR_CLASS(Face,CellEntity,isFace);

  Edges edges() const;
  Volumes volumes() const;
  FaceUse negativeUse() const;
  FaceUse positiveUse() const;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Face_h
