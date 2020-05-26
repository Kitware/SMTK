//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_Edge_h
#define smtk_session_opencascade_Edge_h

#include "smtk/session/opencascade/Shape.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

class Wire;
class Vertex;

class SMTKOPENCASCADESESSION_EXPORT Edge : public Shape
{
public:
  smtkTypeMacro(Edge);
  smtkSuperclassMacro(smtk::session::opencascade::Shape);

  Edge(const std::shared_ptr<smtk::graph::ResourceBase>& rsrc)
    : Shape(rsrc)
  {
  }

  std::vector<std::reference_wrapper<const Wire> > wires() const;
  bool visitWires(const std::function<bool(const Wire&)>& fn) const;
  bool visitWires(const std::function<bool(Wire&)>& fn);

  std::vector<std::reference_wrapper<const Vertex> > vertices() const;
  bool visitVertices(const std::function<bool(const Vertex&)>& fn) const;
  bool visitVertices(const std::function<bool(Vertex&)>& fn);
};
}
}
}

#endif
