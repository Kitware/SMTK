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
/*!\file */

#include "smtk/graph/Component.h"

#include "smtk/resource/Properties.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

class Resource;

class SMTKOPENCASCADESESSION_EXPORT Shape : public smtk::graph::Component
{
public:
  smtkTypeMacro(Shape);
  smtkSuperclassMacro(smtk::graph::Component);
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
};

} // namespace opencascade
} // namespace session
} // namespace smtk

#endif // smtk_session_opencascade_Shape_h
