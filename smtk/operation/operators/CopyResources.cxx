//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/operators/CopyResources.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/io/Logger.h"

#include "smtk/resource/CopyOptions.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"

#include "smtk/operation/Group.h"
#include "smtk/operation/groups/WriterGroup.h"
#include "smtk/operation/operators/CopyResources_xml.h"

#include "nlohmann/json.hpp"

#include <fstream>

using json = nlohmann::json;

namespace smtk
{
namespace operation
{

CopyResources::CopyResources() = default;

bool CopyResources::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  // To copy resources, they must each be able to create an empty clone.
  // Returning a clone does not guarantee copying is possible, but it is
  // a necessary condition and should not be expensive for a reasonable
  // number of inputs
  smtk::resource::CopyOptions options(this->log());
  auto sources = this->parameters()->associations();
  for (const auto& sourceObj : *sources)
  {
    auto source = std::dynamic_pointer_cast<smtk::resource::Resource>(sourceObj);
    if (!source)
    {
      smtkErrorMacro(this->log(), "Non-resource or null association!");
    }
    auto target = source->clone(options);
    if (!target)
    {
      smtkInfoMacro(
        this->log(), "Cannot clone \"" << source->name() << "\" (" << source->id() << ").");
      return false;
    }
  }

  return true;
}

smtk::operation::Operation::Result CopyResources::operateInternal()
{
  smtk::resource::CopyOptions options(this->log());
  auto sources = this->parameters()->associations();
  std::map<smtk::resource::Resource::Ptr, smtk::resource::Resource::Ptr> copies;
  for (const auto& sourceObj : *sources)
  {
    auto source = std::dynamic_pointer_cast<smtk::resource::Resource>(sourceObj);
    if (!source)
    {
      smtkErrorMacro(this->log(), "Non-resource or null association!");
    }
    auto target = source->clone(options);
    if (!target)
    {
      smtkErrorMacro(
        this->log(), "Could not clone \"" << source->name() << "\" (" << source->id() << ").");
      return this->createResult(Outcome::FAILED);
    }
    copies[source] = target;
    if (!target->copyStructure(source, options))
    {
      smtkErrorMacro(
        this->log(),
        "Could not copy structure of \"" << source->name() << "\" (" << source->id() << ").");
      return this->createResult(Outcome::FAILED);
    }
  }
  // Now we have cloned and copied the structure (components, other user data)
  // of each input, we must copy relationship information that may span resources.
  for (const auto& copyPair : copies)
  {
    if (!copyPair.second->copyRelations(copyPair.first, options))
    {
      smtkErrorMacro(
        this->log(),
        "Could not copy relations of "
        "\""
          << copyPair.first->name() << "\" (" << copyPair.first->id() << ").");
      return this->createResult(Outcome::FAILED);
    }
  }

  // Finally, we know that all the copying has succeeded.
  // Create a successful result and add  the created resources to it.
  auto result = this->createResult(Outcome::SUCCEEDED);
  auto resources = result->findResource("resource");
  for (const auto& copyPair : copies)
  {
    resources->appendValue(copyPair.second);
  }

  return result;
}

const char* CopyResources::xmlDescription() const
{
  return CopyResources_xml;
}

} // namespace operation
} // namespace smtk
