//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/markup/operators/SetName.h"

#include "smtk/markup/Resource.h"
#include "smtk/markup/operators/SetName_xml.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/markup/json/jsonResource.h"

#include "smtk/resource/json/Helper.h"

#include "smtk/common/Paths.h"

namespace smtk
{
namespace markup
{

SetName::Result SetName::operateInternal()
{
  auto object = this->parameters()->associations()->value();
  auto name = this->parameters()->findString("name")->value();

  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  if (auto resource = std::dynamic_pointer_cast<smtk::markup::Resource>(object))
  {
    resource->setName(name);
    result->findResource("resourcesModified")->appendValue(resource);
  }
  else if (auto component = std::dynamic_pointer_cast<smtk::markup::Component>(object))
  {
    component->setName(name);
    result->findComponent("modified")->appendValue(component);
  }
  else
  {
    smtkErrorMacro(this->log(), "Unknown object " << object);
    result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  return result;
}

const char* SetName::xmlDescription() const
{
  return SetName_xml;
}

} // namespace markup
} // namespace smtk
