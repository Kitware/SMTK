//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/markup/operators/Create.h"

#include "smtk/markup/Create_xml.h"
#include "smtk/markup/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/common/Paths.h"

#include <fstream>

using namespace smtk::model;

namespace smtk
{
namespace markup
{

Create::Result Create::operateInternal()
{
  // We may be given an optional location:
  auto resource = smtk::markup::Resource::create();
  auto filenameItem = this->parameters()->findFile("filename");
  if (filenameItem->isEnabled())
  {
    auto filename = filenameItem->value();
    if (!filename.empty())
    {
      resource->setLocation(filename);
    }
  }

  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  result->findResource("resource")->appendValue(resource);

  return result;
}

const char* Create::xmlDescription() const
{
  return Create_xml;
}

void Create::markModifiedResources(Result& result)
{
  auto resource = result->findResource("resource")->value();
  if (resource)
  {
    resource->setClean(true);
  }
}

smtk::resource::ResourcePtr create(
  const smtk::common::UUID& uid,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  Create::Ptr create = Create::create();
  create->setManagers(managers);
  Create::Result result = create->operate();
  if (result->findInt("outcome")->value() != static_cast<int>(Create::Outcome::SUCCEEDED))
  {
    return smtk::resource::ResourcePtr();
  }
  auto resource = result->findResource("resource")->value();
  if (resource)
  {
    resource->setId(uid);
  }
  return resource;
}

} // namespace markup
} // namespace smtk
