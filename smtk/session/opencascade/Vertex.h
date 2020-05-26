//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_Vertex_h
#define smtk_session_opencascade_Vertex_h

#include "smtk/session/opencascade/Shape.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

class Edge;

class SMTKOPENCASCADESESSION_EXPORT Vertex : public Shape
{
public:
  smtkTypeMacro(Vertex);
  smtkSuperclassMacro(smtk::session::opencascade::Shape);

  Vertex(const std::shared_ptr<smtk::graph::ResourceBase>& rsrc)
    : Shape(rsrc)
  {
  }

  std::vector<std::reference_wrapper<const Edge> > edges() const;
  bool visitEdges(const std::function<bool(const Edge&)>& fn) const;
  bool visitEdges(const std::function<bool(Edge&)>& fn);
};
}
}
}

#endif
