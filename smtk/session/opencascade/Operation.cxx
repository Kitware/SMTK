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

#include "smtk/session/opencascade/Resource.h"
#include "smtk/session/opencascade/Session.h"
#include "smtk/session/opencascade/Shape.h"

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
    operation::MarkGeometry geom(resource->shared_from_this());
    TopoDS_Iterator iter;
    for (iter.Initialize(*shape); iter.More(); iter.Next())
    {
      TopoDS_Shape childShape = iter.Value();
      if (session->findID(childShape).isNull())
      {
        auto node = resource->create<Shape>();
        std::ostringstream nodeName;
        auto shapeType = childShape.ShapeType();
        std::string topologyType = TopAbs::ShapeTypeToString(shapeType);
        std::transform(topologyType.begin(), topologyType.end(), topologyType.begin(),
          [](unsigned char c) { return std::tolower(c); });
        nodeName << topologyType << " " << session->shapeCounters()[shapeType]++;
        node->setName(nodeName.str());
        session->addStorage(node->id(), childShape);
        geom.markModified(node);
        // created->appendValue(node); // This is problematic for large models.
        this->iterateChildren(*node, result);
      }
    }
  }
}

} // namespace opencascade
} // namespace session
} // namespace smtk
