#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"

#include "smtk/model/Vertex.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Tessellation.h"

namespace smtk {
  namespace model {

/**\brief Return all the uses of this edge.
  *
  * Note that while creating an edge in a Manager instance will
  * create 2 "empty" arrangements to reference EdgeUse relations,
  * those arrangements will not by default point to valid
  * edge uses (i.e., creating an edge does not create a pair
  * of edge-uses for that edge).
  * Because Edge::edgeUses() returns only valid uses, you may
  * not obtain any uses -- or you may obtain an odd number of
  * uses even though edge-uses usually come in pairs.
  */
smtk::model::EdgeUses Edge::edgeUses() const
{
  return this->uses<EdgeUses>();
}

/**\brief Return the vertices which bound this edge.
  *
  * This method is provided for convenience.
  * The *proper* way to obtain vertices along an edge
  * is to fetch the chains of an edge-use and
  * add vertices from the respective vertex-use
  * records along each chain.
  * This allows "interrupted edges" (i.e., edges with
  * holes in their interior) just as edge-loops and
  * face-shells can have voids in their interior.
  */
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
smtk::common::Vector3d Edge::coordinates() const
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
        return smtk::common::Vector3d(coords[0], coords[1], coords[2]);
        }
      }
    }
  return smtk::common::Vector3d().setConstant(std::numeric_limits<double>::quiet_NaN());
}
*/

  } // namespace model
} // namespace smtk
