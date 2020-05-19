//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/operators/CreateBox.h"
#include "smtk/session/opencascade/Resource.h"
#include "smtk/session/opencascade/Session.h"
#include "smtk/session/opencascade/Shape.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/resource/Manager.h"

#include "smtk/common/Paths.h"

#include "smtk/session/opencascade/CreateBox_xml.h"

#include "BRepPrimAPI_MakeBox.hxx"
#include "BRepTools.hxx"
#include "BRep_Builder.hxx"
#include "TopoDS_Iterator.hxx"
#include "TopoDS_Shape.hxx"
#include "gp_Pnt.hxx" // "gp/gp_Pnt.hxx"

#include "smtk/session/opencascade/Vertex.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

std::size_t CreateBox::s_counter = 0;

CreateBox::Result CreateBox::operateInternal()
{
  smtk::session::opencascade::Resource::Ptr resource;
  smtk::session::opencascade::Session::Ptr session;
  auto result = this->createResult(CreateBox::Outcome::FAILED);
  this->prepareResourceAndSession(result, resource, session);

  auto centerItem = this->parameters()->findDouble("center");
  auto sizeItem = this->parameters()->findDouble("size");

  auto created = result->findComponent("created");

  gp_Pnt p1;
  gp_Pnt p2;
  for (int ii = 0; ii < 3; ++ii)
  {
    double ctr = centerItem->value(ii);
    double sz = sizeItem->value(ii);
    p1.SetCoord(ii + 1, ctr - sz / 2.0);
    p2.SetCoord(ii + 1, ctr + sz / 2.0);
  }
  TopoDS_Solid shape = BRepPrimAPI_MakeBox(p1, p2);
  BRep_Builder aBuilder;
  aBuilder.Add(resource->compound(), shape);
  std::ostringstream boxName;
  boxName << "Box " << CreateBox::s_counter++;
  auto topNode = this->createNode(shape, resource.get(), true, boxName.str());
  this->iterateChildren(*topNode, result);
  created->appendValue(topNode->shared_from_this());

  // std::cout
  //   << "  Added " << " " << topNode->id() << " " << topNode->name()
  //   << " " << TopAbs::ShapeTypeToString(shape.ShapeType()) << "\n";

  if (true)
  {
    std::set<const Vertex*> vertices;

    std::size_t indent = 2;
    std::function<bool(const Shape&)> visitor;
    visitor = [&indent, &visitor, &vertices](const Shape& shape) {
      if (const Vertex* vertex = dynamic_cast<const Vertex*>(&shape))
      {
        vertices.insert(vertex);
      }

      for (std::size_t ii = 0; ii < indent; ++ii)
      {
        std::cout << " ";
      }
      std::cout << shape.typeName() << " " << shape.name() << "\n";

      indent += 2;
      shape.visit<Children>(visitor);
      // for (const Shape& subShape : shape.get<SubShapes>())
      // {
      //   visitor(subShape);
      // }
      indent -= 2;
      return false;
    };

    visitor(*topNode);

    std::cout << "visited " << vertices.size() << " vertices" << std::endl;

    visitor = [&indent, &visitor](const Shape& shape) {
      for (std::size_t ii = 0; ii < indent; ++ii)
      {
        std::cout << " ";
      }
      std::cout << shape.typeName() << " " << shape.name() << "\n";

      indent += 2;
      shape.visit<Parents>(visitor);
      // for (const Shape& subShape : shape.get<SubShapes>())
      // {
      //   visitor(subShape);
      // }
      indent -= 2;
      return false;
    };

    visitor(**vertices.begin());
  }

  if (false)
  {
    std::size_t indent = 2;
    std::function<bool(Shape*)> visitor;
    visitor = [&indent, &visitor](Shape* subshape) -> bool {
      for (std::size_t ii = 0; ii < indent; ++ii)
      {
        std::cout << " ";
      }
      std::cout << subshape->typeName() << " " << subshape->name() << "\n";
      indent += 2;
      subshape->visitSubshapes(visitor);
      indent -= 2;
      return false; // do not stop
    };
    visitor(topNode);
  }

  result->findInt("outcome")->setValue(static_cast<int>(CreateBox::Outcome::SUCCEEDED));
  return result;
}

const char* CreateBox::xmlDescription() const
{
  return CreateBox_xml;
}

} // namespace opencascade
} // namespace session
} // namespace smtk
