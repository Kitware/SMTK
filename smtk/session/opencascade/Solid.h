//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_Solid_h
#define smtk_session_opencascade_Solid_h

#include "smtk/session/opencascade/Shape.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

class CompSolid;
class Shell;

class SMTKOPENCASCADESESSION_EXPORT Solid : public Shape
{
public:
  smtkTypeMacro(Solid);
  smtkSuperclassMacro(smtk::session::opencascade::Shape);

  Solid(const std::shared_ptr<smtk::graph::ResourceBase>& rsrc)
    : Shape(rsrc)
  {
  }

  std::vector<std::reference_wrapper<const CompSolid> > compSolids() const;
  bool visitCompSolids(const std::function<bool(const CompSolid&)>& fn) const;
  bool visitCompSolids(const std::function<bool(CompSolid&)>& fn);

  std::vector<std::reference_wrapper<const Shell> > shells() const;
  bool visitShells(const std::function<bool(const Shell&)>& fn) const;
  bool visitShells(const std::function<bool(Shell&)>& fn);
};
}
}
}

#endif
