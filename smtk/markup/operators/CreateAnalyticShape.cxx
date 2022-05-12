//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/markup/operators/CreateAnalyticShape.h"

#include "smtk/markup/Box.h"
#include "smtk/markup/CreateAnalyticShape_xml.h"
#include "smtk/markup/Resource.h"
#include "smtk/markup/Sphere.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

using namespace smtk::model;

namespace smtk
{
namespace markup
{

CreateAnalyticShape::Result CreateAnalyticShape::operateInternal()
{
  auto params = this->parameters();
  auto resource = params->associations()->valueAs<smtk::markup::Resource>();
  CreateAnalyticShape::Result result;
  if (!resource)
  {
    smtkErrorMacro(this->log(), "No resource provided.");
    result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  else
  {
    smtk::string::Token shapeType = params->findString("shape")->value();
    std::shared_ptr<smtk::markup::AnalyticShape> shape;
    if (shapeType == smtk::string::Token("smtk::markup::Box"))
    {
      auto boxCenterItem = params->findDouble("box center");
      auto boxSizeItem = params->findDouble("box size");
      std::array<double, 3> boxCenter{ boxCenterItem->value(0),
                                       boxCenterItem->value(1),
                                       boxCenterItem->value(2) };
      std::array<double, 3> boxSize{ boxSizeItem->value(0),
                                     boxSizeItem->value(1),
                                     boxSizeItem->value(2) };
      std::array<double, 3> boxLo{ boxCenter[0] - std::abs(boxSize[0]),
                                   boxCenter[1] - std::abs(boxSize[1]),
                                   boxCenter[2] - std::abs(boxSize[2]) };
      std::array<double, 3> boxHi{ boxCenter[0] + std::abs(boxSize[0]),
                                   boxCenter[1] + std::abs(boxSize[1]),
                                   boxCenter[2] + std::abs(boxSize[2]) };
      shape = resource->createNode<smtk::markup::Box>(boxLo, boxHi);
    }
    else if (shapeType == smtk::string::Token("smtk::markup::Sphere"))
    {
      auto sphereCenterItem = params->findDouble("sphere center");
      auto sphereSizeItem = params->findDouble("sphere radius");
      std::array<double, 3> sphereCenter{ sphereCenterItem->value(0),
                                          sphereCenterItem->value(1),
                                          sphereCenterItem->value(2) };
      std::array<double, 3> sphereRadii{ std::abs(sphereSizeItem->value(0)),
                                         std::abs(sphereSizeItem->value(1)),
                                         std::abs(sphereSizeItem->value(2)) };
      shape = resource->createNode<smtk::markup::Sphere>(sphereCenter, sphereRadii);
    }
    if (shape)
    {
      std::ostringstream name;
      name << "new " << shape->typeName();
      shape->setName(name.str());
      result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
      result->findComponent("created")->appendValue(shape);
      smtk::operation::MarkGeometry().markModified(shape); // Irrelevant now, but not in the future.
    }
    else
    {
      smtkErrorMacro(this->log(), "Invalid shape " << shapeType.data() << " requested.");
      result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
  }
  return result;
}

const char* CreateAnalyticShape::xmlDescription() const
{
  return CreateAnalyticShape_xml;
}

} // namespace markup
} // namespace smtk
