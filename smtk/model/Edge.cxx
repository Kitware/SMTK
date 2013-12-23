#include "smtk/model/Edge.h"

#include "smtk/model/Vertex.h"
#include "smtk/model/Storage.h"
#include "smtk/model/Tessellation.h"

namespace smtk {
  namespace model {

smtk::model::Vertices Edge::vertices() const
{
  Vertices result;
  Cursors all = this->boundaryEntities(/*dim = */ 0);
  for (Cursors::iterator it = all.begin(); it != all.end(); ++it)
    {
    if (it->isVertex())
      result.push_back(*it);
    }
  return result;
}

/*
smtk::util::Vector3d Edge::coordinates() const
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
