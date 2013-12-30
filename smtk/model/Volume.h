#ifndef __smtk_model_Volume_h
#define __smtk_model_Volume_h

#include "smtk/model/CellEntity.h"

//#include "smtk/util/Eigen.h" // For Vector3d

#include <vector>

namespace smtk {
  namespace model {

class Face;
typedef std::vector<Face> Faces;

/**\brief A cursor subclass that provides methods specific to 0-d vertex cells.
  *
  */
class SMTKCORE_EXPORT Volume : public CellEntity
{
public:
  SMTK_CURSOR_CLASS(Volume,CellEntity,isVolume);

  Faces faces() const;
};

typedef std::vector<Volume> Volumes;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Volume_h
