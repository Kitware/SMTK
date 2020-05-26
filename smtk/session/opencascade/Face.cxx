//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/Face.h"

#include "smtk/session/opencascade/Shell.h"
#include "smtk/session/opencascade/Wire.h"
#include "smtk/session/opencascade/arcs/ChildrenAs.h"
#include "smtk/session/opencascade/arcs/ParentsAs.h"

namespace smtk
{
namespace session
{
namespace opencascade
{
ParentsAs<Shell>::Container Face::shells() const
{
  return get<ParentsAs<Shell> >();
}
bool Face::visitShells(const std::function<bool(const Shell&)>& fn) const
{
  return visit<ParentsAs<Shell> >(fn);
}
bool Face::visitShells(const std::function<bool(Shell&)>& fn)
{
  return visit<ParentsAs<Shell> >(fn);
}

ChildrenAs<Wire>::Container Face::wires() const
{
  return get<ChildrenAs<Wire> >();
}
bool Face::visitWires(const std::function<bool(const Wire&)>& fn) const
{
  return visit<ChildrenAs<Wire> >(fn);
}
bool Face::visitWires(const std::function<bool(Wire&)>& fn)
{
  return visit<ChildrenAs<Wire> >(fn);
}
}
}
}
