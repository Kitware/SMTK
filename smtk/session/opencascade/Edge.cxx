//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/Edge.h"

#include "smtk/session/opencascade/Vertex.h"
#include "smtk/session/opencascade/Wire.h"
#include "smtk/session/opencascade/arcs/ChildrenAs.h"
#include "smtk/session/opencascade/arcs/ParentsAs.h"

namespace smtk
{
namespace session
{
namespace opencascade
{
ParentsAs<Wire>::Container Edge::wires() const
{
  return get<ParentsAs<Wire> >();
}
bool Edge::visitWires(const std::function<bool(const Wire&)>& fn) const
{
  return visit<ParentsAs<Wire> >(fn);
}
bool Edge::visitWires(const std::function<bool(Wire&)>& fn)
{
  return visit<ParentsAs<Wire> >(fn);
}

ChildrenAs<Vertex>::Container Edge::vertices() const
{
  return get<ChildrenAs<Vertex> >();
}
bool Edge::visitVertices(const std::function<bool(const Vertex&)>& fn) const
{
  return visit<ChildrenAs<Vertex> >(fn);
}
bool Edge::visitVertices(const std::function<bool(Vertex&)>& fn)
{
  return visit<ChildrenAs<Vertex> >(fn);
}
}
}
}
