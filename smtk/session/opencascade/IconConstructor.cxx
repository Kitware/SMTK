//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/IconConstructor.h"

#include "smtk/session/opencascade/Resource.h"
#include "smtk/session/opencascade/Session.h"
#include "smtk/session/opencascade/Shape.h"

#include "smtk/common/Color.h"

#include "smtk/view/icons/edge_svg.h"
#include "smtk/view/icons/face_svg.h"
#include "smtk/view/icons/model_svg.h"
#include "smtk/view/icons/vertex_svg.h"
#include "smtk/view/icons/volume_svg.h"

#include <regex>
#include <vector>

namespace smtk
{
namespace session
{
namespace opencascade
{

std::string IconConstructor::svg(const smtk::resource::PersistentObject& object) const
{
  if (auto shape = dynamic_cast<const smtk::session::opencascade::Shape*>(&object))
  {
    auto resource = static_pointer_cast<Resource>(shape->resource());
    auto session = resource->session();
    const TopoDS_Shape* occShape = session->findShape(shape->id());

    switch (occShape->ShapeType())
    {
      case TopAbs_COMPOUND:
        return model_svg;
      case TopAbs_COMPSOLID:
      case TopAbs_SOLID:
        return volume_svg;
      case TopAbs_SHELL:
        return model_svg; // TODO
      case TopAbs_FACE:
        return face_svg;
      case TopAbs_WIRE: // TODO
      case TopAbs_EDGE:
        return edge_svg;
      case TopAbs_VERTEX:
        return vertex_svg;
      case TopAbs_SHAPE: // TODO
      default:
        return "";
    }
  }
  else
  {
    return model_svg;
  }
}
}
}
}
