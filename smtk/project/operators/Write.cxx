//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/project/operators/Write.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/ResourceItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/io/Logger.h"

#include "smtk/operation/operators/WriteResource.h"

#include "smtk/project/Manager.h"

#include "smtk/project/json/jsonProject.h"

#include "smtk/project/Write_xml.h"

#include <iostream>

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

namespace smtk
{
namespace project
{

Write::Result Write::operateInternal()
{
  std::cout << "smtk::project::Write::operateInternal()" << std::endl;
  // Access the project to write.
  smtk::attribute::ReferenceItem::Ptr projectItem = this->parameters()->associations();
  smtk::project::ProjectPtr project = projectItem->valueAs<smtk::project::Project>();

  // Access the file name to write.
  std::string outputFile = project->location();
  boost::filesystem::path outputFilePath(outputFile);
  boost::filesystem::path projectFolderPath = outputFilePath.parent_path();
  boost::filesystem::path resourcesFolderPath = projectFolderPath / "resources";

  // Construct a WriteResource operation to write all of the project's resources.
  smtk::operation::WriteResource::Ptr write =
    project->operations().manager()->create<smtk::operation::WriteResource>();
  if (!write)
  {
    smtkErrorMacro(this->log(), "Cannot create WriteResource operation.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Create project and project/resources folders if needed
  if (!boost::filesystem::exists(resourcesFolderPath))
  {
    if (!boost::filesystem::create_directories(resourcesFolderPath))
    {
      smtkErrorMacro(
        this->log(),
        "Failed to create project resources directory: " << resourcesFolderPath.string() << ".");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
  }
  else if (!boost::filesystem::is_directory(resourcesFolderPath))
  {
    smtkErrorMacro(
      this->log(), "Resource path is not a folder: " << resourcesFolderPath.string() << ".");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  std::set<smtk::resource::Resource*> unassignedResources;
  std::map<std::string, std::string> resourceDictionary;

  // Write the modified resources.
  for (auto& resource : project->resources())
  {
    const std::string& role = detail::role(resource);

    if (!resource->clean())
    {
      // Reset the write operation's associations.
      write->parameters()->associations()->reset();

      if (resource->location().empty())
      {
        unassignedResources.insert(resource.get());
        std::string filename = role + ".smtk";
        boost::filesystem::path location = resourcesFolderPath / filename;
        resource->setLocation(location.string());
      }

      write->parameters()->associate(resource);
      smtk::operation::Operation::Result writeResult = write->operate();
      if (
        writeResult->findInt("outcome")->value() !=
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
      {
        // An error message should already enter the logger from the local operation.
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
    }

    resourceDictionary[resource->location()] = role + ".smtk";
  }

  // We now write the project's smtk file.
  {
    nlohmann::json j = project;
    if (j.is_null())
    {
      smtkErrorMacro(log(), "Unable to serialize project to json object.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    std::string fileContents = j.dump(2);
    std::ofstream file(outputFile);
    file << fileContents;
    file.close();
  }
  // Construct a result object.
  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // Unassign any temporary locations we assigned.
  if (!unassignedResources.empty())
  {
    for (auto resource : unassignedResources)
    {
      std::cout << "Unassigned resource type" << resource->typeName() << std::endl;
      resource->setLocation("");
    }
  }

  return result;
}

const char* Write::xmlDescription() const
{
  return Write_xml;
}

bool write(const smtk::resource::ResourcePtr& resource)
{
  Write::Ptr write = Write::create();
  write->parameters()->associate(resource);
  Write::Result result = write->operate();
  return (result->findInt("outcome")->value() == static_cast<int>(Write::Outcome::SUCCEEDED));
}
} // namespace project
} // namespace smtk
