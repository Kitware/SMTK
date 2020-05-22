//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/CompSolid.h"

#include "smtk/session/opencascade/Compound.h"
#include "smtk/session/opencascade/Solid.h"
#include "smtk/session/opencascade/arcs/ChildrenAs.h"
#include "smtk/session/opencascade/arcs/ParentsAs.h"

namespace smtk
{
namespace session
{
namespace opencascade
{
ParentsAs<Compound>::Container CompSolid::compounds() const
{
  return get<ParentsAs<Compound> >();
}
bool CompSolid::visitCompounds(const std::function<bool(const Compound&)>& fn) const
{
  return visit<ParentsAs<Compound> >(fn);
}
bool CompSolid::visitCompounds(const std::function<bool(Compound&)>& fn)
{
  return visit<ParentsAs<Compound> >(fn);
}

ChildrenAs<Solid>::Container CompSolid::solids() const
{
  return get<ChildrenAs<Solid> >();
}
bool CompSolid::visitSolids(const std::function<bool(const Solid&)>& fn) const
{
  return visit<ChildrenAs<Solid> >(fn);
}
bool CompSolid::visitSolids(const std::function<bool(Solid&)>& fn)
{
  return visit<ChildrenAs<Solid> >(fn);
}
}
}
}
