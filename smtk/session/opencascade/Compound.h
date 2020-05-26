//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_Compound_h
#define smtk_session_opencascade_Compound_h

#include "smtk/session/opencascade/Shape.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

class CompSolid;

class SMTKOPENCASCADESESSION_EXPORT Compound : public Shape
{
public:
  smtkTypeMacro(Compound);
  smtkSuperclassMacro(smtk::session::opencascade::Shape);

  Compound(const std::shared_ptr<smtk::graph::ResourceBase>& rsrc)
    : Shape(rsrc)
  {
  }

  std::vector<std::reference_wrapper<const CompSolid> > compSolids() const;
  bool visitCompSolids(const std::function<bool(const CompSolid&)>&) const;
  bool visitCompSolids(const std::function<bool(CompSolid&)>&);
};
}
}
}

#endif
