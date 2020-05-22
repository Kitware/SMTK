//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_Shell_h
#define smtk_session_opencascade_Shell_h

#include "smtk/session/opencascade/Shape.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

class Face;
class Solid;

class SMTKOPENCASCADESESSION_EXPORT Shell : public Shape
{
public:
  smtkTypeMacro(Shell);
  smtkSuperclassMacro(smtk::session::opencascade::Shape);

  Shell(const std::shared_ptr<smtk::graph::ResourceBase>& rsrc)
    : Shape(rsrc)
  {
  }

  std::vector<std::reference_wrapper<const Solid> > solids() const;
  bool visitSolids(const std::function<bool(const Solid&)>& fn) const;
  bool visitSolids(const std::function<bool(Solid&)>& fn);

  std::vector<std::reference_wrapper<const Face> > faces() const;
  bool visitFaces(const std::function<bool(const Face&)>& fn) const;
  bool visitFaces(const std::function<bool(Face&)>& fn);
};
}
}
}

#endif
