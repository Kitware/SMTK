//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/project/Project.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"
#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

#include "boost/filesystem.hpp"
#include "nlohmann/json.hpp"

#include <exception>
#include <fstream>

using json = nlohmann::json;

namespace
{
std::string PROJECT_FILENAME = ".cmbproject";
int UNABLE_TO_OPERATE =
  static_cast<int>(smtk::operation::Operation::Outcome::UNABLE_TO_OPERATE);       // 0
int FAILED = static_cast<int>(smtk::operation::Operation::Outcome::FAILED);       // 2
int SUCCEEDED = static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED); // 3
}

namespace smtk
{
namespace project
{
Project::Project()
{
}

Project::~Project()
{
  this->close();
}

smtk::resource::ResourcePtr Project::getResourceByRole(
  const std::string& typeName, const std::string& role) const
{
  if (!this->m_resourceManager)
  {
    return smtk::resource::ResourcePtr();
  }

  for (auto& resourceInfo : this->m_resourceInfos)
  {
    if ((resourceInfo.m_typeName != typeName) || (resourceInfo.m_role != role))
    {
      continue;
    }

    if (this->m_resourceManager)
    {
      return this->m_resourceManager->get(resourceInfo.m_uuid);
    }
  }

  // If we reached here, resource not found
  return smtk::resource::ResourcePtr();
}

void Project::setCoreManagers(
  smtk::resource::ManagerPtr resourceManager, smtk::operation::ManagerPtr operationManager)
{
  if (!this->m_resourceInfos.empty())
  {
    throw std::runtime_error("Cannot change core managers on open project");
  }
  this->m_resourceManager = resourceManager;
  this->m_resourceManager->registerResource<smtk::attribute::Resource>();
  this->m_operationManager = operationManager;
}

int Project::build(smtk::attribute::AttributePtr specification)
{
  // Note that specification is the "new-project" definition in NewProject.sbt.
  // Validate starting conditions
  if (!specification->isValid())
  {
    std::cerr << "ERROR: invalid project specification" << std::endl;
    return FAILED;
  }
  if (!this->m_resourceManager)
  {
    std::cerr << "Cannot create project - resource manager not set" << std::endl;
    return FAILED;
  }
  if (!this->m_operationManager)
  {
    std::cerr << "Cannot create project - operation manager not set" << std::endl;
    return FAILED;
  }

  // Todo check that at least 1 resource specified?

  // Initialize return value
  int outcome = static_cast<int>(smtk::operation::Operation::Outcome::UNKNOWN);

  this->m_name = specification->findString("project-name")->value(0);
  this->m_directory = specification->findDirectory("project-directory")->value(0);

  // Initialize project directory
  boost::filesystem::path boostDirectory(this->m_directory);
  if (boost::filesystem::exists(boostDirectory))
  {
    boost::filesystem::remove_all(boostDirectory);
  }
  boost::filesystem::create_directory(boostDirectory);

  // Import the model resource
  ResourceInfo modelInfo;
  auto modelFileItem = specification->findFile("model-file");
  if (modelFileItem->isEnabled())
  {
    std::string modelPath = modelFileItem->value(0);
    bool copyNativeModel = specification->findVoid("copy-model-file")->isEnabled();
    outcome = this->importModel(modelPath, copyNativeModel, modelInfo);
    if (outcome != SUCCEEDED)
    {
      return outcome;
    }
    this->m_resourceInfos.push_back(modelInfo);
  } // if (modelFileItem emabled)

  // Load the attribute template
  ResourceInfo attInfo;
  auto attFileItem = specification->findFile("simulation-template");
  if (attFileItem->isEnabled())
  {
    std::string attPath = attFileItem->value(0);
    outcome = this->importAttributeTemplate(attPath, attInfo);
    if (outcome != SUCCEEDED)
    {
      return outcome;
    }
    this->m_resourceInfos.push_back(attInfo);
  } // if (attFileItem enabled)

  // Link attribute resource to model resource
  if (!modelInfo.m_uuid.isNull() && !attInfo.m_uuid.isNull())
  {
    auto attResource = this->m_resourceManager->get(attInfo.m_uuid);
    auto modelResource = this->m_resourceManager->get(modelInfo.m_uuid);
    smtk::dynamic_pointer_cast<smtk::attribute::Resource>(attResource)->associate(modelResource);
  }

  // Write .cmbproject file
  this->writeProjectFile();

  return SUCCEEDED;
}

int Project::save() const
{
  boost::filesystem::path boostDirectory(this->m_directory);
  auto logger = smtk::io::Logger();
  // For now, saving a project consists of saving its resources.
  // Later may include saving project metadata
  for (auto& info : this->m_resourceInfos)
  {
    auto resource = this->m_resourceManager->get(info.m_uuid);
    auto path = boostDirectory / boost::filesystem::path(info.m_filename);
    if (resource->typeName() == "smtk::attribute::Resource")
    {
      // Always save attribute resource, since clean() method not reliable
      auto writer = smtk::io::AttributeWriter();
      auto attResource = smtk::dynamic_pointer_cast<smtk::attribute::Resource>(resource);
      bool err = writer.write(attResource, path.string(), logger);
      if (err)
      {
        return FAILED;
      }
    } // if
    else if (!resource->clean())
    {
      auto writer = this->m_operationManager->create("smtk::operation::WriteResource");
      writer->parameters()->associate(resource);
      writer->parameters()->find("filename")->setIsEnabled(true);
      writer->parameters()->findFile("filename")->setValue(0, path.string());
      auto result = writer->operate();
      int outcome = result->findInt("outcome")->value(0);
      if (outcome != SUCCEEDED)
      {
        return outcome;
      }
    } // else if
  }   // for (info)

  return SUCCEEDED;
} // save()

int Project::close()
{
  if (!this->m_resourceManager)
  {
    return UNABLE_TO_OPERATE;
  }

  // Release resources
  for (auto& resourceInfo : this->m_resourceInfos)
  {
    auto resourcePtr = this->m_resourceManager->get(resourceInfo.m_uuid);
    this->m_resourceManager->remove(resourcePtr);
    resourcePtr.reset();
  }
  this->m_resourceInfos.clear();
  this->m_name.clear();
  this->m_directory.clear();

  return SUCCEEDED;
} // close()

int Project::open(const std::string& location)
{
  // Setup path to directory and .cmbproject file
  boost::filesystem::path directoryPath;
  boost::filesystem::path dotfilePath;

  boost::filesystem::path inputPath(location);
  if (!boost::filesystem::exists(inputPath))
  {
    std::cerr << "Specified project path not found" << std::endl;
    return FAILED;
  }

  if (boost::filesystem::is_directory(inputPath))
  {
    directoryPath = inputPath;
    dotfilePath = directoryPath / boost::filesystem::path(PROJECT_FILENAME);
    if (!boost::filesystem::exists(dotfilePath))
    {
      std::cerr << "No \"" << PROJECT_FILENAME << "\" file in specified directory" << std::endl;
      return FAILED;
    }
  }
  else if (inputPath.filename().string() != PROJECT_FILENAME)
  {
    std::cerr << "Invalid project filename, should be \"" << PROJECT_FILENAME << "\"" << std::endl;
    return FAILED;
  }
  else
  {
    dotfilePath = inputPath;
    directoryPath = inputPath.parent_path();
  }

  // Load project file
  std::ifstream dotFile;
  dotFile.open(dotfilePath.string().c_str(), std::ios_base::in | std::ios_base::ate);
  if (!dotFile)
  {
    std::cerr << "Failed loading \"" << PROJECT_FILENAME << "\" file" << std::endl;
    return FAILED;
  }
  auto fileSize = dotFile.tellg();
  std::string dotFileContents;
  dotFileContents.reserve(fileSize);

  dotFile.seekg(0, std::ios_base::beg);
  dotFileContents.assign(
    (std::istreambuf_iterator<char>(dotFile)), std::istreambuf_iterator<char>());

  ProjectInfo projectInfo;
  try
  {
    auto j = json::parse(dotFileContents);
    projectInfo = j;
  }
  catch (std::exception /* e */)
  {
    std::cerr << "Error parsing \"" << PROJECT_FILENAME << "\" file" << std::endl;
    return FAILED;
  }

  this->m_name = projectInfo.m_name;
  this->m_directory = projectInfo.m_directory;
  this->m_resourceInfos = projectInfo.m_resourceInfos;

  return this->loadResources(directoryPath.string());
} // open()

int Project::importModel(const std::string& importPath, bool copyNativeModel, ResourceInfo& resInfo)
{
  std::cerr << "Loading model " << importPath << std::endl;
  int outcome = static_cast<int>(smtk::operation::Operation::Outcome::UNKNOWN); // return value

  boost::filesystem::path boostImportPath(importPath);
  boost::filesystem::path boostDirectory(this->m_directory);

  // Create the import operator
  auto importOp = this->m_operationManager->create("smtk::operation::ImportResource");
  if (!importOp)
  {
    throw std::runtime_error("No import operator");
  }

  // Run the import operator
  importOp->parameters()->findFile("filename")->setValue(importPath);
  auto importOpResult = importOp->operate();
  outcome = importOpResult->findInt("outcome")->value(0);
  if (outcome != SUCCEEDED)
  {
    return outcome;
  }
  auto modelResource = importOpResult->findResource("resource")->value(0);

  // Save the new resource
  auto modelWriter = this->m_operationManager->create("smtk::operation::WriteResource");
  modelWriter->parameters()->associate(modelResource);
  modelWriter->parameters()->find("filename")->setIsEnabled(true);

  // boost::filesystem::path boostModelPath(modelPath);
  auto modelFilename = boostImportPath.stem();
  auto smtkFilename = modelFilename.replace_extension("smtk");
  auto smtkPath = boostDirectory / smtkFilename;
  modelWriter->parameters()->findFile("filename")->setValue(0, smtkPath.string());

  auto result = modelWriter->operate();
  outcome = result->findInt("outcome")->value(0);
  if (outcome != SUCCEEDED)
  {
    return outcome;
  }

  // Update resource info
  resInfo.m_filename = smtkFilename.string();
  resInfo.m_identifier = "model";
  resInfo.m_importLocation = importPath;
  resInfo.m_role = "default";
  resInfo.m_typeName = modelResource->typeName();
  resInfo.m_uuid = modelResource->id();

  // Copy the import (native) model file
  if (copyNativeModel)
  {
    // auto copyPath = boostDirectory / modelFilename;
    auto copyPath = boostDirectory / boostImportPath.filename();
    boost::filesystem::copy_file(importPath, copyPath);
    resInfo.m_importLocation = copyPath.string();
  }

  return SUCCEEDED;
} // importModel()

int Project::importAttributeTemplate(const std::string& location, ResourceInfo& resInfo)
{
  std::cerr << "Loading templateFile: " << location << std::endl;

  // Read from specified location
  auto attResource = smtk::attribute::Resource::create();
  smtk::io::AttributeReader reader;
  smtk::io::Logger logger;
  bool readErr = reader.read(attResource, location, true, logger);
  if (readErr)
  {
    std::cerr << logger.convertToString(true) << std::endl;
    attResource.reset();
    return FAILED;
  }
  this->m_resourceManager->add(attResource);

  // Update resource info
  if (resInfo.m_identifier == "")
  {
    resInfo.m_identifier = "default";
  }
  resInfo.m_filename = resInfo.m_identifier + ".sbi";
  resInfo.m_importLocation = location;
  if (resInfo.m_role == "")
  {
    resInfo.m_role = "default";
  }
  resInfo.m_typeName = attResource->typeName();
  resInfo.m_uuid = attResource->id();

  // Save to project directory
  auto writer = smtk::io::AttributeWriter();
  boost::filesystem::path boostDirectory(this->m_directory);
  boost::filesystem::path sbiPath = boostDirectory / boost::filesystem::path(resInfo.m_filename);
  logger.reset();
  bool writeErr = writer.write(attResource, sbiPath.string(), logger);
  if (writeErr)
  {
    return FAILED;
  }

  return SUCCEEDED;
} // importAttributeTemplate()

int Project::writeProjectFile() const
{
  // Init ProjectInfo structure
  ProjectInfo projInfo;
  projInfo.m_name = this->m_name;
  projInfo.m_directory = this->m_directory;
  projInfo.m_resourceInfos = this->m_resourceInfos;

  // Convert to json
  nlohmann::json jInfo = projInfo;

  // Write file
  std::ofstream projectFile;
  auto path =
    boost::filesystem::path(this->m_directory) / boost::filesystem::path(PROJECT_FILENAME);
  projectFile.open(path.string().c_str(), std::ofstream::out | std::ofstream::trunc);
  if (!projectFile)
  {
    return FAILED;
  }
  projectFile << jInfo.dump(2) << "\n";
  projectFile.close();

  return SUCCEEDED;
}

int Project::loadResources(const std::string& path)
{
  // Part of opening project from disk
  boost::filesystem::path directoryPath(path);

  for (auto& info : this->m_resourceInfos)
  {
    auto filePath = directoryPath / boost::filesystem::path(info.m_filename);
    auto inputPath = filePath.string();
    if (info.m_typeName == "smtk::attribute::Resource")
    {
      auto attResource = smtk::attribute::Resource::create();
      attResource->setId(info.m_uuid);
      smtk::io::AttributeReader reader;
      smtk::io::Logger logger;
      bool err = reader.read(attResource, inputPath, true, logger);
      if (err)
      {
        std::cerr << logger.convertToString(true) << std::endl;
        attResource.reset();
        return FAILED;
      }
      this->m_resourceManager->add(attResource);
    } // if (attribute resource)
    else
    {
      // Create a read operator
      auto readOp = this->m_operationManager->create("smtk::operation::ReadResource");
      if (!readOp)
      {
        throw std::runtime_error("No read operator");
      }
      readOp->parameters()->findFile("filename")->setValue(inputPath);
      auto readOpResult = readOp->operate();

      // Test for success
      int outcome = readOpResult->findInt("outcome")->value(0);
      if (outcome != SUCCEEDED)
      {
        std::cerr << "Error loading resource from: " << inputPath << std::endl;
        return outcome;
      }
    } // else
  }   // for (info)

  return SUCCEEDED;
} // loadResources()

} // namespace project
} // namespace smtk
