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

#include "smtk/project/operators/Read_xml.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <fstream>
#include <string>

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

  // manually reset the "location" string so that smtk::project::from_json() can properly translate
  //   resource paths to being relative (rather than absolute)
  j["location"] = filename;

  // Transcribe project data into the project
  smtk::project::from_json(j, project);

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
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

smtk::resource::ResourcePtr read(
  const std::string& filename,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  Read::Ptr read = Read::create();
  read->setManagers(managers);
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
