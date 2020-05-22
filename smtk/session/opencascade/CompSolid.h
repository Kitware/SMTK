//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_CompSolid_h
#define smtk_session_opencascade_CompSolid_h

#include "smtk/session/opencascade/Shape.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

class Compound;
class Solid;

class SMTKOPENCASCADESESSION_EXPORT CompSolid : public Shape
{
public:
  smtkTypeMacro(CompSolid);
  smtkSuperclassMacro(smtk::session::opencascade::Shape);

  CompSolid(const std::shared_ptr<smtk::graph::ResourceBase>& rsrc)
    : Shape(rsrc)
  {
  }

  std::vector<std::reference_wrapper<const Compound> > compounds() const;
  bool visitCompounds(const std::function<bool(const Compound&)>& fn) const;
  bool visitCompounds(const std::function<bool(Compound&)>& fn);

  std::vector<std::reference_wrapper<const Solid> > solids() const;
  bool visitSolids(const std::function<bool(const Solid&)>& fn) const;
  bool visitSolids(const std::function<bool(Solid&)>& fn);
};
}
}
}

#endif
