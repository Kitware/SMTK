//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"

#include "smtk/model/Face.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Vertex.h"

#include "smtk/common/GeometryUtilities.h"

namespace smtk
{
namespace model
{

/**\brief Return all the uses of this edge.
  *
  * Note that while creating an edge in a Resource instance will
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

/**\brief Return the edge use-record with the given \a sense and \a orientation, creating it if needed.
  *
  * The \a orientation specifies whether the edge use-record refers to the positive (\a orientation true)
  * or negative direction of the edge.
  *
  * The \a sense specifies the context in which the edge is used.
  * For models whose parametric dimension is 2, the \a sense should always be 0.
  * For models with higher dimension (e.g., volumetric models), each loop
  * referring to an edge should have a unique combination of (sense, orientation).
  */
EdgeUse Edge::findOrAddEdgeUse(Orientation orientation, int sense)
{
  smtk::model::Resource::Ptr resource(m_resource.lock());
  if (this->isValid())
  {
    return EdgeUse(
      resource,
      resource->findCreateOrReplaceCellUseOfSenseAndOrientation(m_entity, sense, orientation));
  }
  return EdgeUse();
}

/**\brief Return the faces which this edge bounds.
  *
  * This method is provided for convenience.
  * The *proper* way to obtain faces bounded by an edge
  * is to fetch the loops of an edge-use and
  * add faces from the respective face-use
  * records along each loop.
  */
smtk::model::Faces Edge::faces() const
{
  Faces result;
  EntityRefs all = this->bordantEntities(/*ofDimension = */ 2);
  for (EntityRefs::iterator it = all.begin(); it != all.end(); ++it)
  {
    if (it->isFace())
      result.emplace_back(*it);
  }
  return result;
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
  EntityRefs all = this->boundaryEntities(/*ofDimension = */ 0);
  for (EntityRefs::iterator it = all.begin(); it != all.end(); ++it)
  {
    if (it->isVertex())
      result.emplace_back(*it);
  }

  // Now attempt to get the order correct for cases we can handle.
  if (result.size() == 2 && result[0] != result[1])
  {
    const Tessellation* etess = this->hasTessellation();
    if (etess)
    {
      int i0, i1;
      if (etess->vertexIdsOfPolylineEndpoints(0, i0, i1))
      {
        double v0d =
          smtk::common::distance2(&(etess->coords()[3 * i0]), &(result[0].coordinates()[0]));
        double v1d =
          smtk::common::distance2(&(etess->coords()[3 * i1]), &(result[0].coordinates()[0]));
        if (v0d > v1d)
        { // swap the vertices
          smtk::model::Vertex tmp = result[0];
          result[0] = result[1];
          result[1] = tmp;
        }
      }
    }
  }
  return result;
}

bool Edge::isPeriodic() const
{
  Vertices endpts = this->vertices();
  return (endpts.size() < 2);
}

/*
smtk::common::Vector3d Edge::coordinates() const
{
  if (this->isValid())
    {
    ResourcePtr resource = this->resource();
    UUIDWithTessellation tessRec =
      resource->tessellations().find(m_entity);
    if (tessRec != resource->tessellations().end())
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
