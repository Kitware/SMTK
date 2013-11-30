#ifndef __smtk_model_Vertex_h
#define __smtk_model_Vertex_h

#include "smtk/model/CellEntity.h"

#include "smtk/util/Eigen.h" // For Vector3d

namespace smtk {
  namespace model {

class SMTKCORE_EXPORT Vertex : public CellEntity
{
public:
  SMTK_CURSOR_CLASS(Vertex,CellEntity,isVertex);

  //smtk::util::Vector3d coordinates() const;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Vertex_h
