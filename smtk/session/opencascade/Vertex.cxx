//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/Vertex.h"

#include "smtk/session/opencascade/Edge.h"
#include "smtk/session/opencascade/arcs/ParentsAs.h"

namespace smtk
{
namespace session
{
namespace opencascade
{
ParentsAs<Edge>::Container Vertex::edges() const
{
  return get<ParentsAs<Edge> >();
}
bool Vertex::visitEdges(const std::function<bool(const Edge&)>& fn) const
{
  return visit<ParentsAs<Edge> >(fn);
}
bool Vertex::visitEdges(const std::function<bool(Edge&)>& fn)
{
  return visit<ParentsAs<Edge> >(fn);
}
}
}
}
