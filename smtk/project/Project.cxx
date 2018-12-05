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
#include "smtk/common/TypeName.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/operators/ImportResource.h"
#include "smtk/operation/operators/ReadResource.h"
#include "smtk/operation/operators/WriteResource.h"
#include "smtk/project/json/jsonProjectDescriptor.h"
#include "smtk/resource/Manager.h"

#include "boost/filesystem.hpp"

#include <exception>
#include <fstream>

namespace
{
std::string PROJECT_FILENAME = ".smtkproject";
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

std::vector<smtk::resource::ResourcePtr> Project::getResources() const
{
  std::vector<smtk::resource::ResourcePtr> resourceList;

  for (auto& rd : this->m_resourceDescriptors)
  {
    auto resource = this->m_resourceManager->get(rd.m_uuid);
    resourceList.push_back(resource);
  }

  return resourceList;
}

void Project::setCoreManagers(
  smtk::resource::ManagerPtr resourceManager, smtk::operation::ManagerPtr operationManager)
{
  if (!this->m_resourceDescriptors.empty())
  {
    throw std::runtime_error("Cannot change core managers on open project");
  }
  this->m_resourceManager = resourceManager;
  this->m_resourceManager->registerResource<smtk::attribute::Resource>();
  this->m_operationManager = operationManager;
}

bool Project::build(smtk::attribute::AttributePtr specification, smtk::io::Logger& logger,
  bool replaceExistingDirectory)
{
  // Note that specification is the "new-project" definition in NewProject.sbt.
  // Validate starting conditions
  if (!specification->isValid())
  {
    smtkErrorMacro(logger, "invalid project specification");
    return false;
  }

  // (Future) check that at least 1 resource specified?

  this->m_name = specification->findString("project-name")->value(0);
  this->m_directory = specification->findDirectory("project-directory")->value(0);

  // Check if project directory already exists
  boost::filesystem::path boostDirectory(this->m_directory);
  if (boost::filesystem::exists(boostDirectory))
  {
    if (replaceExistingDirectory)
    {
      boost::filesystem::remove_all(boostDirectory);
    }
    else
    {
      smtkErrorMacro(
        logger, "Cannot create project in existing directory: \"" << this->m_directory << "\"");
      return false;
    }

  } // if (directory already exists)
  boost::filesystem::create_directory(boostDirectory);

  // Import the model resource
  ResourceDescriptor modelDescriptor;
  auto modelFileItem = specification->findFile("model-file");
  if (modelFileItem->isEnabled())
  {
    std::string modelPath = modelFileItem->value(0);
    bool copyNativeModel = specification->findVoid("copy-model-file")->isEnabled();
    if (!this->importModel(modelPath, copyNativeModel, modelDescriptor, logger))
    {
      return false;
    }
    this->m_resourceDescriptors.push_back(modelDescriptor);
  } // if (modelFileItem emabled)

  // Load the attribute template
  ResourceDescriptor attDescriptor;
  auto attFileItem = specification->findFile("simulation-template");
  if (attFileItem->isEnabled())
  {
    std::string attPath = attFileItem->value(0);
    bool success = this->importAttributeTemplate(attPath, attDescriptor, logger);
    if (!success)
    {
      return false;
    }
    this->m_resourceDescriptors.push_back(attDescriptor);
  } // if (attFileItem enabled)

  // Link attribute resource to model resource
  if (!modelDescriptor.m_uuid.isNull() && !attDescriptor.m_uuid.isNull())
  {
    auto attResource = this->m_resourceManager->get(attDescriptor.m_uuid);
    auto modelResource = this->m_resourceManager->get(modelDescriptor.m_uuid);
    smtk::dynamic_pointer_cast<smtk::attribute::Resource>(attResource)->associate(modelResource);
  }

  // Write .cmbproject file
  if (!this->writeProjectFile(logger))
  {
    return false;
  }

  return true;
}

bool Project::save(smtk::io::Logger& logger) const
{
  boost::filesystem::path boostDirectory(this->m_directory);

  // For now, saving a project consists of saving its resources.
  // Later may include saving project metadata
  for (auto& rd : this->m_resourceDescriptors)
  {
    auto resource = this->m_resourceManager->get(rd.m_uuid);
    auto path = boostDirectory / boost::filesystem::path(rd.m_filename);
    if (resource->typeName() == smtk::common::typeName<smtk::attribute::Resource>())
    {
      // Always save attribute resource, since clean() method not reliable
      auto writer = smtk::io::AttributeWriter();
      auto attResource = smtk::dynamic_pointer_cast<smtk::attribute::Resource>(resource);
      bool err = writer.write(attResource, path.string(), logger);
      if (err)
      {
        return false;
      }
    } // if
    else if (!resource->clean())
    {
      auto writer = this->m_operationManager->create<smtk::operation::WriteResource>();
      writer->parameters()->associate(resource);
      writer->parameters()->find("filename")->setIsEnabled(true);
      writer->parameters()->findFile("filename")->setValue(0, path.string());
      auto result = writer->operate();
      int outcome = result->findInt("outcome")->value(0);
      if (outcome != SUCCEEDED)
      {
        smtkErrorMacro(logger, "Error writing resource file " << path.string());
        return false;
      }
    } // else if
  }   // for (rd)

  return true;
} // save()

bool Project::close()
{
  // Release resources
  for (auto& rd : this->m_resourceDescriptors)
  {
    auto resourcePtr = this->m_resourceManager->get(rd.m_uuid);
    this->m_resourceManager->remove(resourcePtr);
  }
  this->m_resourceDescriptors.clear();
  this->m_name.clear();
  this->m_directory.clear();

  return true;
} // close()

bool Project::open(const std::string& location, smtk::io::Logger& logger)
{
  // Setup path to directory and .cmbproject file
  boost::filesystem::path directoryPath;
  boost::filesystem::path dotfilePath;

  boost::filesystem::path inputPath(location);
  if (!boost::filesystem::exists(inputPath))
  {
    smtkErrorMacro(logger, "Specified project path not found" << location);
    return false;
  }

  if (boost::filesystem::is_directory(inputPath))
  {
    directoryPath = inputPath;
    dotfilePath = directoryPath / boost::filesystem::path(PROJECT_FILENAME);
    if (!boost::filesystem::exists(dotfilePath))
    {
      smtkErrorMacro(logger, "No \"" << PROJECT_FILENAME << "\" file in specified directory");
      return false;
    }
  }
  else if (inputPath.filename().string() != PROJECT_FILENAME)
  {
    smtkErrorMacro(logger, "Invalid project filename, should be \"" << PROJECT_FILENAME << "\"");
    return false;
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
    smtkErrorMacro(logger, "Failed loading \"" << PROJECT_FILENAME << "\" file");
    return false;
  }
  auto fileSize = dotFile.tellg();
  std::string dotFileContents;
  dotFileContents.reserve(fileSize);

  dotFile.seekg(0, std::ios_base::beg);
  dotFileContents.assign(
    (std::istreambuf_iterator<char>(dotFile)), std::istreambuf_iterator<char>());

  ProjectDescriptor descriptor;
  try
  {
    parse_json(dotFileContents, descriptor); // (static function in jsonProjectDescriptor.h)
  }
  catch (std::exception& ex)
  {
    smtkErrorMacro(logger, "Error parsing \"" << PROJECT_FILENAME << "\" file");
    smtkErrorMacro(logger, ex.what());
    return false;
  }

  this->m_name = descriptor.m_name;
  this->m_directory = descriptor.m_directory;
  this->m_resourceDescriptors = descriptor.m_resourceDescriptors;

  return this->loadResources(directoryPath.string(), logger);
} // open()

bool Project::importModel(const std::string& importPath, bool copyNativeModel,
  ResourceDescriptor& descriptor, smtk::io::Logger& logger)
{
  smtkDebugMacro(logger, "Loading model " << importPath);

  boost::filesystem::path boostImportPath(importPath);
  boost::filesystem::path boostDirectory(this->m_directory);

  // Create the import operator
  auto importOp = this->m_operationManager->create<smtk::operation::ImportResource>();
  if (!importOp)
  {
    smtkErrorMacro(logger, "Import operator not found");
    return false;
  }

  int outcome;

  // Run the import operator
  importOp->parameters()->findFile("filename")->setValue(importPath);
  auto importOpResult = importOp->operate();
  outcome = importOpResult->findInt("outcome")->value(0);
  if (outcome != SUCCEEDED)
  {
    smtkErrorMacro(logger, "Error importing file " << importPath);
    return false;
  }
  auto modelResource = importOpResult->findResource("resource")->value(0);

  // Save the new resource
  auto modelWriter = this->m_operationManager->create<smtk::operation::WriteResource>();
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
    smtkErrorMacro(logger, "Error writing resource to file " << smtkPath.string());
    return false;
  }

  // Update resource info
  descriptor.m_filename = smtkFilename.string();
  descriptor.m_identifier = "model";
  descriptor.m_importLocation = importPath;
  descriptor.m_typeName = modelResource->typeName();
  descriptor.m_uuid = modelResource->id();

  // Copy the import (native) model file
  if (copyNativeModel)
  {
    // auto copyPath = boostDirectory / modelFilename;
    auto copyPath = boostDirectory / boostImportPath.filename();
    boost::filesystem::copy_file(importPath, copyPath);
    descriptor.m_importLocation = copyPath.string();
  }

  return true; // success
} // importModel()

bool Project::importAttributeTemplate(
  const std::string& location, ResourceDescriptor& descriptor, smtk::io::Logger& logger)
{
  smtkDebugMacro(logger, "Loading templateFile: " << location);

  // Read from specified location
  auto attResource = smtk::attribute::Resource::create();
  smtk::io::AttributeReader reader;
  bool readErr = reader.read(attResource, location, true, logger);
  if (readErr)
  {
    return false; // invert from representing "error" to representing "success"
  }
  this->m_resourceManager->add(attResource);

  // Update descriptor
  if (descriptor.m_identifier == "")
  {
    descriptor.m_identifier = "default";
  }
  descriptor.m_filename = descriptor.m_identifier + ".sbi";
  descriptor.m_importLocation = location;
  descriptor.m_typeName = attResource->typeName();
  descriptor.m_uuid = attResource->id();

  // Save to project directory
  auto writer = smtk::io::AttributeWriter();
  boost::filesystem::path boostDirectory(this->m_directory);
  boost::filesystem::path sbiPath = boostDirectory / boost::filesystem::path(descriptor.m_filename);
  bool writeErr = writer.write(attResource, sbiPath.string(), logger);
  if (writeErr)
  {
    return false; // invert
  }

  return true;
} // importAttributeTemplate()

bool Project::writeProjectFile(smtk::io::Logger& logger) const
{
  // Init ProjectDescriptor structure
  ProjectDescriptor descriptor;
  descriptor.m_name = this->m_name;
  descriptor.m_directory = this->m_directory;
  descriptor.m_resourceDescriptors = this->m_resourceDescriptors;

  // Get json string
  std::string dotFileContents =
    dump_json(descriptor); // (static function in jsonProjectDescriptor.h)

  // Write file
  std::ofstream projectFile;
  auto path =
    boost::filesystem::path(this->m_directory) / boost::filesystem::path(PROJECT_FILENAME);
  projectFile.open(path.string().c_str(), std::ofstream::out | std::ofstream::trunc);
  if (!projectFile)
  {
    smtkErrorMacro(logger, "Unable to write to project file " << path.string());
    return false;
  }
  projectFile << dotFileContents << "\n";
  projectFile.close();

  return true;
}

bool Project::loadResources(const std::string& path, smtk::io::Logger& logger)
{
  // Part of opening project from disk
  boost::filesystem::path directoryPath(path);

  for (auto& descriptor : this->m_resourceDescriptors)
  {
    auto filePath = directoryPath / boost::filesystem::path(descriptor.m_filename);
    auto inputPath = filePath.string();
    if (descriptor.m_typeName == smtk::common::typeName<smtk::attribute::Resource>())
    {
      auto attResource = smtk::attribute::Resource::create();
      attResource->setId(descriptor.m_uuid);
      smtk::io::AttributeReader reader;
      bool err = reader.read(attResource, inputPath, true, logger);
      if (err)
      {
        return false;
      }
      this->m_resourceManager->add(attResource);
    } // if (attribute resource)
    else
    {
      // Create a read operator
      auto readOp = this->m_operationManager->create<smtk::operation::ReadResource>();
      if (!readOp)
      {
        smtkErrorMacro(logger, "Read Resource operator not found");
        return false;
      }
      readOp->parameters()->findFile("filename")->setValue(inputPath);
      auto readOpResult = readOp->operate();

      // Test for success
      int outcome = readOpResult->findInt("outcome")->value(0);
      if (outcome != SUCCEEDED)
      {
        smtkErrorMacro(logger, "Error loading resource from: " << inputPath);
        return false;
      }
    } // else
  }   // for (info)

  return true;
} // loadResources()

} // namespace project
} // namespace smtk
