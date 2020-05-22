//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/Compound.h"

#include "smtk/session/opencascade/CompSolid.h"
#include "smtk/session/opencascade/arcs/ChildrenAs.h"

namespace smtk
{
namespace session
{
namespace opencascade
{
ChildrenAs<CompSolid>::Container Compound::compSolids() const
{
  return get<ChildrenAs<CompSolid> >();
}
bool Compound::visitCompSolids(const std::function<bool(const CompSolid&)>& fn) const
{
  return visit<ChildrenAs<CompSolid> >(fn);
}
bool Compound::visitCompSolids(const std::function<bool(CompSolid&)>& fn)
{
  return visit<ChildrenAs<CompSolid> >(fn);
}
}
}
}
