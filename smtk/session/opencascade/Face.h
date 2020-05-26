//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_Face_h
#define smtk_session_opencascade_Face_h

#include "smtk/session/opencascade/Shape.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

class Shell;
class Wire;

class SMTKOPENCASCADESESSION_EXPORT Face : public Shape
{
public:
  smtkTypeMacro(Face);
  smtkSuperclassMacro(smtk::session::opencascade::Shape);

  Face(const std::shared_ptr<smtk::graph::ResourceBase>& rsrc)
    : Shape(rsrc)
  {
  }

  std::vector<std::reference_wrapper<const Shell> > shells() const;
  bool visitShells(const std::function<bool(const Shell&)>& fn) const;
  bool visitShells(const std::function<bool(Shell&)>& fn);

  std::vector<std::reference_wrapper<const Wire> > wires() const;
  bool visitWires(const std::function<bool(const Wire&)>& fn) const;
  bool visitWires(const std::function<bool(Wire&)>& fn);
};
}
}
}

#endif
