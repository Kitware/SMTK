#include "smtk/model/Vertex.h"

#include "smtk/model/Storage.h"
#include "smtk/model/Tessellation.h"

namespace smtk {
  namespace model {

/*
smtk::util::Vector3d Vertex::coordinates() const
{
  if (this->isValid())
    {
    UUIDWithTessellation tessRec =
      this->m_storage->tessellations().find(this->m_entity);
    if (tessRec != this->m_storage->tessellations().end())
      {
      if (!tessRec->second.coords().empty())
        {
        double* coords = &tessRec->second.coords()[0];
        return smtk::util::Vector3d(coords[0], coords[1], coords[2]);
        }
      }
    }
  return smtk::util::Vector3d().setConstant(std::numeric_limits<double>::quiet_NaN());
}
*/

  } // namespace model
} // namespace smtk
