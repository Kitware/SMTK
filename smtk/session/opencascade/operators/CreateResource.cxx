//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/operators/CreateResource.h"
#include "smtk/session/opencascade/Resource.h"
#include "smtk/session/opencascade/Session.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/resource/Manager.h"

#include "smtk/common/Paths.h"

#include "smtk/session/opencascade/CreateResource_xml.h"

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

CreateResource::Result CreateResource::operateInternal()
{
  smtk::session::opencascade::Resource::Ptr resource;
  smtk::session::opencascade::Session::Ptr session;
  auto result = this->createResult(CreateResource::Outcome::FAILED);
  this->prepareResourceAndSession(result, resource, session);
  if (!resource || !session)
  {
    return result;
  }

  auto fileItem = this->parameters()->findFile("location");
  if (fileItem && fileItem->isEnabled() && fileItem->isSet())
  {
    resource->setLocation(fileItem->value());
  }

  result->findInt("outcome")->setValue(static_cast<int>(CreateResource::Outcome::SUCCEEDED));
  return result;
}

const char* CreateResource::xmlDescription() const
{
  return CreateResource_xml;
}

} // namespace opencascade
} // namespace session
} // namespace smtk
