//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_Wire_h
#define smtk_session_opencascade_Wire_h

#include "smtk/session/opencascade/Shape.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

class Face;
class Edge;

class SMTKOPENCASCADESESSION_EXPORT Wire : public Shape
{
public:
  smtkTypeMacro(Wire);
  smtkSuperclassMacro(smtk::session::opencascade::Shape);

  Wire(const std::shared_ptr<smtk::graph::ResourceBase>& rsrc)
    : Shape(rsrc)
  {
  }

  std::vector<std::reference_wrapper<const Face> > faces() const;
  bool visitFaces(const std::function<bool(const Face&)>& fn) const;
  bool visitFaces(const std::function<bool(Face&)>& fn);

  std::vector<std::reference_wrapper<const Edge> > edges() const;
  bool visitEdges(const std::function<bool(const Edge&)>& fn) const;
  bool visitEdges(const std::function<bool(Edge&)>& fn);
};
}
}
}

#endif
