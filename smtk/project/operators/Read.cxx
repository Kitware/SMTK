//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/project/operators/Read.h"

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

#include "smtk/operation/operators/ReadResource.h"

#include "smtk/project/Manager.h"

#include "smtk/project/json/jsonProject.h"

#include "smtk/project/Read_xml.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

namespace smtk
{
namespace project
{

void Read::markModifiedResources(Read::Result& result)
{
  int outcome = result->findInt("outcome")->value();
  if (outcome != static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    return;
  }

  auto resourceItem = result->findResource("resource");
  auto resource = resourceItem->value();
  if (resource != nullptr)
  {
    resource->setClean(true);
  }
}

Read::Result Read::operateInternal()
{
  std::string filename = this->parameters()->findFile("filename")->value();

  std::ifstream file(filename);
  if (!file.good())
  {
    smtkErrorMacro(log(), "Cannot read file \"" << filename << "\".");
    file.close();
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  nlohmann::json j;
  try
  {
    j = nlohmann::json::parse(file);
  }
  catch (...)
  {
    smtkErrorMacro(log(), "Cannot parse file \"" << filename << "\".");
    file.close();
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  file.close();

  // Access the project's id
  std::string projectIdStr = j.at("id");
  smtk::common::UUID projectId(projectIdStr);

  // Create a new project for the import
  boost::filesystem::path projectFilePath(filename);
  auto project = this->projectManager()->create(j.at("type").get<std::string>());
  if (project == nullptr)
  {
    smtkErrorMacro(log(), "project of type " << j.at("type") << " was not created.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  project->setId(projectId);
  project->setLocation(filename);

  // Get folder path - might be needed to load resources
  boost::filesystem::path projectPath = projectFilePath.parent_path();

  // Transcribe project data into the project
  smtk::project::from_json(j, project);

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // For now, load all project resources
  j = j["resources"];
  for (json::const_iterator it = j["resources"].begin(); it != j["resources"].end(); ++it)
  {
    boost::filesystem::path path(it->at("location").get<std::string>());
    if (path.is_relative())
    {
      path = projectPath / path;
    }
    smtk::resource::ResourcePtr resource =
      project->resources().manager()->read(it->at("type").get<std::string>(), path.string());
    if (!resource)
    {
      smtkErrorMacro(
        log(),
        "Cannot read resource type \"" << it->at("type").get<std::string>() << "\" at location \""
                                       << it->at("location").get<std::string>() << "\".");

      result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
      continue;
    }
    resource->setClean(true);
    project->resources().add(resource, detail::role(resource));
  }

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->setValue(project);
  }

  return result;
}

const char* Read::xmlDescription() const
{
  return Read_xml;
}

smtk::resource::ResourcePtr read(const std::string& filename)
{
  Read::Ptr read = Read::create();
  read->parameters()->findFile("filename")->setValue(filename);
  Read::Result result = read->operate();
  if (result->findInt("outcome")->value() != static_cast<int>(Read::Outcome::SUCCEEDED))
  {
    return smtk::resource::ResourcePtr();
  }
  return result->findResource("resource")->value();
}
} // namespace project
} // namespace smtk
