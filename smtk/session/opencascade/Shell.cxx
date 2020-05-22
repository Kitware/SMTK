//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/Shell.h"

#include "smtk/session/opencascade/Face.h"
#include "smtk/session/opencascade/Solid.h"
#include "smtk/session/opencascade/arcs/ChildrenAs.h"
#include "smtk/session/opencascade/arcs/ParentsAs.h"

namespace smtk
{
namespace session
{
namespace opencascade
{
ParentsAs<Solid>::Container Shell::solids() const
{
  return get<ParentsAs<Solid> >();
}
bool Shell::visitSolids(const std::function<bool(const Solid&)>& fn) const
{
  return visit<ParentsAs<Solid> >(fn);
}
bool Shell::visitSolids(const std::function<bool(Solid&)>& fn)
{
  return visit<ParentsAs<Solid> >(fn);
}

ChildrenAs<Face>::Container Shell::faces() const
{
  return get<ChildrenAs<Face> >();
}
bool Shell::visitFaces(const std::function<bool(const Face&)>& fn) const
{
  return visit<ChildrenAs<Face> >(fn);
}
bool Shell::visitFaces(const std::function<bool(Face&)>& fn)
{
  return visit<ChildrenAs<Face> >(fn);
}
}
}
}
