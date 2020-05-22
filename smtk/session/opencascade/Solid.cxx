//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/Solid.h"

#include "smtk/session/opencascade/CompSolid.h"
#include "smtk/session/opencascade/Shell.h"
#include "smtk/session/opencascade/arcs/ChildrenAs.h"
#include "smtk/session/opencascade/arcs/ParentsAs.h"

namespace smtk
{
namespace session
{
namespace opencascade
{
ParentsAs<CompSolid>::Container Solid::compSolids() const
{
  return get<ParentsAs<CompSolid> >();
}
bool Solid::visitCompSolids(const std::function<bool(const CompSolid&)>& fn) const
{
  return visit<ParentsAs<CompSolid> >(fn);
}
bool Solid::visitCompSolids(const std::function<bool(CompSolid&)>& fn)
{
  return visit<ParentsAs<CompSolid> >(fn);
}

ChildrenAs<Shell>::Container Solid::shells() const
{
  return get<ChildrenAs<Shell> >();
}
bool Solid::visitShells(const std::function<bool(const Shell&)>& fn) const
{
  return visit<ChildrenAs<Shell> >(fn);
}
bool Solid::visitShells(const std::function<bool(Shell&)>& fn)
{
  return visit<ChildrenAs<Shell> >(fn);
}
}
}
}
