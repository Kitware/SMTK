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
  auto topNode = resource->create<Shape>();
  session->addShape(topNode->id(), shape);
  operation::MarkGeometry geom(resource);
  created->appendValue(topNode);
  geom.markModified(topNode);
  std::ostringstream boxName;
  boxName << "Box " << CreateBox::s_counter++;
  topNode->setName(boxName.str());
  // std::cout
  //   << "  Added " << " " << topNode->id() << " " << topNode->name()
  //   << " " << TopAbs::ShapeTypeToString(shape.ShapeType()) << "\n";

  this->iterateChildren(*topNode, result);

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
