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
#include "smtk/resource/json/Helper.h"
#include "smtk/task/json/Helper.h"

#include "smtk/view/Manager.h"
#include "smtk/view/UIElementState.h"

#include "smtk/project/operators/Write_xml.h"

#include <fstream>
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
  // Access the project to write.
  smtk::attribute::ReferenceItem::Ptr projectItem = this->parameters()->associations();
  smtk::project::ProjectPtr project = projectItem->valueAs<smtk::project::Project>();

  // Get the project file (path) and setup folders
  std::string outputFile = project->location();
  if (outputFile.empty())
  {
    smtkErrorMacro(this->log(), "Error Cannot write project because location not specified.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
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

  // Write the modified resources.
  for (const auto& resource : project->resources())
  {
    const std::string& role = detail::role(resource);

    if (!resource->clean())
    {
      // Reset the write operation's associations.
      write->parameters()->associations()->reset();

      if (resource->location().empty())
      {
        std::string filename = role + "-" + resource->id().toString() + ".smtk";
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
  }

  // Push helpers onto the stack so `to_json()` methods can reference
  // state external to the objects they are serializing.
  auto& resourceHelper = smtk::resource::json::Helper::pushInstance(project);
  resourceHelper.setManagers(this->managers());
  smtk::task::json::Helper::pushInstance(project->taskManager(), this->managers());

  // We now write the project's smtk file.
  {
    nlohmann::json j = project;
    if (j.is_null())
    {
      smtkErrorMacro(log(), "Unable to serialize project to json object.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    // Save the JSON configurations of all the UI Elements in the
    // View Manager.
    auto managers = this->managers();
    if (managers)
    {
      auto viewMngr = this->managers()->get<smtk::view::Manager::Ptr>();
      if (viewMngr)
      {
        nlohmann::json js;
        auto& elementStateMap = viewMngr->elementStateMap();
        for (auto& element : elementStateMap)
        {
          js[element.first.data()] = element.second->configuration();
        }
        j["ui_state"] = js;
      }
    }
    std::string fileContents = j.dump(2);
    std::ofstream file(outputFile);
    file << fileContents;
    file.close();
  }

  // Pop helpers above from the stack.
  smtk::resource::json::Helper::popInstance();
  smtk::task::json::Helper::popInstance();

  // Reset the project's clean flag
  project->setClean(true);

  // Construct a result object.
  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  return result;
}

const char* Write::xmlDescription() const
{
  return Write_xml;
}

bool write(
  const smtk::resource::ResourcePtr& resource,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  Write::Ptr write = Write::create();
  write->setManagers(managers);
  write->parameters()->associate(resource);
  Write::Result result = write->operate();
  return (result->findInt("outcome")->value() == static_cast<int>(Write::Outcome::SUCCEEDED));
}
} // namespace project
} // namespace smtk
