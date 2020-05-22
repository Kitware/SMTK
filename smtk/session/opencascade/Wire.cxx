//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/Wire.h"

#include "smtk/session/opencascade/Edge.h"
#include "smtk/session/opencascade/Face.h"
#include "smtk/session/opencascade/arcs/ChildrenAs.h"
#include "smtk/session/opencascade/arcs/ParentsAs.h"

namespace smtk
{
namespace session
{
namespace opencascade
{
ParentsAs<Face>::Container Wire::faces() const
{
  return get<ParentsAs<Face> >();
}
bool Wire::visitFaces(const std::function<bool(const Face&)>& fn) const
{
  return visit<ParentsAs<Face> >(fn);
}
bool Wire::visitFaces(const std::function<bool(Face&)>& fn)
{
  return visit<ParentsAs<Face> >(fn);
}

ChildrenAs<Edge>::Container Wire::edges() const
{
  return get<ChildrenAs<Edge> >();
}
bool Wire::visitEdges(const std::function<bool(const Edge&)>& fn) const
{
  return visit<ChildrenAs<Edge> >(fn);
}
bool Wire::visitEdges(const std::function<bool(Edge&)>& fn)
{
  return visit<ChildrenAs<Edge> >(fn);
}
}
}
}
