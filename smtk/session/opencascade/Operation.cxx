//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/Operation.h"

#include "smtk/session/opencascade/CompSolid.h"
#include "smtk/session/opencascade/Compound.h"
#include "smtk/session/opencascade/Edge.h"
#include "smtk/session/opencascade/Face.h"
#include "smtk/session/opencascade/Resource.h"
#include "smtk/session/opencascade/Session.h"
#include "smtk/session/opencascade/Shape.h"
#include "smtk/session/opencascade/Shell.h"
#include "smtk/session/opencascade/Solid.h"
#include "smtk/session/opencascade/Vertex.h"
#include "smtk/session/opencascade/Wire.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/resource/Manager.h"

#include "smtk/common/Paths.h"

#include "BRepTools.hxx"
#include "BRep_Builder.hxx"
#include "TopoDS_Iterator.hxx"
#include "TopoDS_Shape.hxx"

namespace smtk
{
namespace session
{
namespace opencascade
{

void Operation::prepareResourceAndSession(
  Result& result, std::shared_ptr<Resource>& resource, std::shared_ptr<Session>& session)
{
  auto assoc = this->parameters()->associations();
  if (assoc && assoc->isEnabled() && assoc->isSet(0))
  {
    resource = dynamic_pointer_cast<Resource>(assoc->value(0));
    if (!resource)
    {
      auto comp = dynamic_pointer_cast<smtk::resource::Component>(assoc->value(0));
      if (comp)
      {
        resource = dynamic_pointer_cast<Resource>(comp->resource());
      }
    }
    if (resource)
    {
      session = resource->session();
    }
  }

  // Create a new resource for the import if needed.
  if (!resource)
  {
    auto manager = this->specification()->manager();
    if (manager)
    {
      resource = manager->create<smtk::session::opencascade::Resource>();
    }
    else
    {
      resource = smtk::session::opencascade::Resource::create();
    }
    auto resultResources = result->findResource("resource");
    if (resultResources)
    {
      resultResources->setValue(resource);
    }
  }
  if (!session)
  {
    session = smtk::session::opencascade::Session::create();
    resource->setSession(session);
  }
}

Shape* Operation::createNode(
  TopoDS_Shape& shape, Resource* resource, bool mark, const std::string& name)
{
  auto shapeType = shape.ShapeType();
  Shape::Ptr node;
  switch (shapeType)
  {
    case TopAbs_COMPOUND:
      node = resource->create<Compound>();
      break;
    case TopAbs_COMPSOLID:
      mark = false;
      node = resource->create<CompSolid>();
      break;
    case TopAbs_SOLID:
      mark = false;
      node = resource->create<Solid>();
      break;
    case TopAbs_SHELL:
      mark = false;
      node = resource->create<Shell>();
      break;
    case TopAbs_FACE:
      node = resource->create<Face>();
      break;
    case TopAbs_WIRE:
      mark = false;
      node = resource->create<Wire>();
      break;
    case TopAbs_EDGE:
      node = resource->create<Edge>();
      break;
    case TopAbs_VERTEX:
      node = resource->create<Vertex>();
      break;
    case TopAbs_SHAPE: // fall through
    default:
      mark = false;
      node = resource->create<Shape>();
      break;
  }
  std::string nname;
  if (name.empty())
  {
    std::ostringstream nodeName;
    std::string topologyType = TopAbs::ShapeTypeToString(shapeType);
    std::transform(topologyType.begin(), topologyType.end(), topologyType.begin(),
      [](unsigned char c) { return std::tolower(c); });
    nodeName << topologyType << " " << resource->session()->shapeCounters()[shapeType]++;
    nname = nodeName.str();
  }
  else
  {
    nname = name;
  }
  node->setName(nname);
  resource->session()->addShape(node->id(), shape);
  if (mark)
  {
    operation::MarkGeometry(resource->shared_from_this()).markModified(node);
  }
  return node.get();
}

void Operation::iterateChildren(Shape& parent, Result& result)
{
  Resource* resource = dynamic_cast<Resource*>(parent.resource().get());
  if (!resource)
  {
    return;
  }

  // auto created = result->findComponent("created");
  auto session = resource->session();
  auto shape = session->findShape(parent.id());
  if (shape)
  {
    TopoDS_Iterator iter;
    for (iter.Initialize(*shape); iter.More(); iter.Next())
    {
      TopoDS_Shape childShape = iter.Value();
      if (session->findID(childShape).isNull())
      {
        auto node = this->createNode(childShape, resource, true);
        // created->appendValue(node); // This is problematic for large models.
        this->iterateChildren(*node, result);
      }
    }
  }
}

} // namespace opencascade
} // namespace session
} // namespace smtk
