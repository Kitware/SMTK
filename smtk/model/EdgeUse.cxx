//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/EdgeUse.h"

#include "smtk/model/Chain.h"
#include "smtk/model/Edge.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/VertexUse.h"

namespace smtk
{
namespace model
{

// The face-use on which this edge-use lies (if any).
FaceUse EdgeUse::faceUse() const
{
  return this->loop().boundingUseEntity().as<FaceUse>();
}

/**\brief The loop of the face associated with this edge use (if any) or an invalid entity.
  *
  */
Loop EdgeUse::loop() const
{
  return this->boundingShellEntity().as<Loop>();
}

/// The (parent) underlying edge of this use
Edge EdgeUse::edge() const
{
  return this->cell().as<Edge>();
}

/**\brief Return the vertex-chains associated with this edge-use.
  *
  * Unlike traditional cellular homology literature, a chain
  * in this context is the 0-d analogue of a 1-d edge-loop
  * or a 2-d face-shell. It is considered to be "closed"
  * in the way that ShellEntities of other dimensions are
  * closed.
  *
  * An edge may have multiple top-level chains (i.e., multiple
  * disconnected components on the same curve) and each
  * chain may have subchains (i.e., "voids").
  * The order in which chains are listed is considered
  * significant.
  */
Chains EdgeUse::chains() const
{
  return this->shellEntities<Chains>();
}

static void getVertexUsesOfChainBFS(const Chain& c, VertexUses& result)
{
  VertexUses chainEndPts = c.uses<VertexUses>();
  VertexUses::iterator curEndPt = chainEndPts.begin();
  if (curEndPt != chainEndPts.end())
  {
    result.insert(result.end(), *curEndPt);
    ++curEndPt;
  }

  // Now handle any subchains
  Chains subs = c.containedChains();
  for (Chains::iterator it = subs.begin(); it != subs.end(); ++it)
  {
    if (it->isValid())
    {
      getVertexUsesOfChainBFS(*it, result);
    }
  }

  // Now add remaining vertex uses from the current chain
  // (Really there should only be one. Period.)
  for (; curEndPt != chainEndPts.end(); ++curEndPt)
  {
    result.insert(result.end(), *curEndPt);
  }
}

/// Ordered list of vertex uses for this edge use.
VertexUses EdgeUse::vertexUses() const
{
  VertexUses result;
  Chains toplevel = this->chains();
  for (Chains::iterator it = toplevel.begin(); it != toplevel.end(); ++it)
  {
    if (it->isValid())
    {
      getVertexUsesOfChainBFS(*it, result);
    }
  }
  return result;
}

// Ordered list of vertices in the sense of this edge use.
Vertices EdgeUse::vertices() const
{
  Vertices result;
  VertexUses uses = this->vertexUses();
  for (VertexUses::iterator it = uses.begin(); it != uses.end(); ++it)
  {
    result.push_back(it->vertex());
  }
  // FIXME: If the model does not create vertex uses then this
  //        fetch vertices from the edge and shuffle them according
  //        to the orientation of the use.
  return result;
}

/// The next edge use around this edge.
EdgeUse EdgeUse::ccwUse() const
{
  ManagerPtr mgr = this->manager();
  // Find the offset into HAS_USE arrangements for this edge-use:
  //int curUse = mgr->findCellHasUseWithSense(this->m_entity, this->sense());
  // Now ask for the next valid, cyclic HAS_USE arrangement.
  //int nxtUse mgr->findAdjacentArrangement(HAS_USE, curUse, +1);
  // Return the relation specified by the arrangement.
  // return this->relationFromArrangement(HAS_USE, nxtUse, int offset).as<EdgeUse>();
  EdgeUse use;
  return use;
}

/// The previous edge use around the edge.
EdgeUse EdgeUse::cwUse() const
{
  EdgeUse use;
  return use;
}

} // namespace model
} // namespace smtk
