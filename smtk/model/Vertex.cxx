#include "smtk/model/Vertex.h"

#include "smtk/model/Edge.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Tessellation.h"

namespace smtk {
  namespace model {

smtk::model::Edges Vertex::edges() const
{
  Edges result;
  Cursors all = this->bordantEntities(/*dim = */ 1);
  for (Cursors::iterator it = all.begin(); it != all.end(); ++it)
    {
    if (it->isEdge())
      result.push_back(*it);
    }
  return result;
}

/*
smtk::util::Vector3d Vertex::coordinates() const
{
  if (this->isValid())
    {
    UUIDWithTessellation tessRec =
      this->m_manager->tessellations().find(this->m_entity);
    if (tessRec != this->m_manager->tessellations().end())
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
