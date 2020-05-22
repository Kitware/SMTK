//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_Shape_h
#define smtk_session_opencascade_Shape_h

#include "smtk/graph/Component.h"

#include "smtk/resource/Properties.h"

#include "smtk/session/opencascade/Exports.h"

#include <TopAbs_ShapeEnum.hxx>

class TopoDS_Shape;

namespace smtk
{
namespace session
{
namespace opencascade
{

class Resource;

/**\brief The basic topological entity of OpenCASCADE is a shape.
  *
  * This is an SMTK component that represents OCC shapes.
  */
class SMTKOPENCASCADESESSION_EXPORT Shape : public smtk::graph::Component
{
public:
  using Visitor = std::function<bool(Shape*)>;

  smtkTypeMacro(Shape);
  smtkSuperclassMacro(smtk::graph::Component);

  static constexpr decltype(TopAbs_SHAPE) OCC_ShapeType = TopAbs_SHAPE;

  Shape(const std::shared_ptr<smtk::graph::ResourceBase>& rsrc)
    : Component(rsrc)
  {
  }

  std::string name() const override
  {
    const auto& props = this->properties();
    if (props.contains<std::string>("name"))
    {
      return this->properties().at<std::string>("name");
    }
    return this->Superclass::name();
  }
  virtual void setName(const std::string& name)
  {
    this->properties().get<std::string>()["name"] = name;
  }

  const TopoDS_Shape* data() const;
  TopoDS_Shape* data();

  /// Return the parent resource as a session::opencascade::Resource, not a resource::Resource.
  Resource* occResource() const;

  /// Invoke \a visitor on every subshape until visitor returns true (to terminate early).
  void visitSubshapes(Visitor visitor);
};
}
}
}

#endif
