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
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/common/TypeName.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/model/EntityTypeBits.h"
#include "smtk/model/Resource.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/operators/ImportResource.h"
#include "smtk/operation/operators/ReadResource.h"
#include "smtk/operation/operators/WriteResource.h"
#include "smtk/project/json/jsonProjectDescriptor.h"

#ifdef SMTK_PYTHON_ENABLED
#include "smtk/operation/operators/ImportPythonOperation.h"
#endif

#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"

#include <algorithm> // for std::transform
#include <exception>

#ifndef NDEBUG
#include <iostream>
#endif

namespace
{
std::string PROJECT_FILENAME = ".smtkproject";
int SUCCEEDED = static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED); // 3
} // namespace

namespace smtk
{
namespace project
{
Project::Project() = default;

Project::~Project()
{
  this->close();
}

std::vector<smtk::resource::ResourcePtr> Project::resources() const
{
  std::vector<smtk::resource::ResourcePtr> resourceList; // return value

  auto resManager = m_resourceManager.lock();
  if (!resManager)
  {
    return resourceList;
  }

  for (const auto& rd : m_resourceDescriptors)
  {
    if (rd.m_uuid.isNull())
    {
      continue;
    }
    auto resource = resManager->get(rd.m_uuid);
    if (!resource)
    {
      continue;
    }

    resourceList.push_back(resource);
  }

  return resourceList;
}

bool Project::clean() const
{
  auto resourceList = this->resources();
  for (const auto& resource : resourceList)
  {
    if (not resource->clean())
    {
      return false;
    }
  }
  return true;
}

std::string Project::importLocation(smtk::resource::ResourcePtr res) const
{
  auto resId = res->id();
  for (const auto& descriptor : m_resourceDescriptors)
  {
    if (descriptor.m_uuid == resId)
    {
      return descriptor.m_importLocation;
    }
  }

  // (else)
  return std::string();
}

bool Project::addModel(
  const std::string& location,
  const std::string& identifier,
  bool copyNativeFile,
  bool useVTKSession)
{
  // Project must not already have a model with the same identifier
  auto model = this->findResource<smtk::model::Resource>(identifier);
  if (model != nullptr)
  {
    return false;
  }

  // Create descriptor
  ResourceDescriptor modelDescriptor;
  auto& logger = smtk::io::Logger::instance();
  if (!this->importModel(location, copyNativeFile, modelDescriptor, useVTKSession, logger))
  {
#ifndef NDEBUG
    std::cerr << logger.convertToString() << std::endl;
#endif
    return false;
  }
  modelDescriptor.m_identifier = identifier;
  m_resourceDescriptors.push_back(modelDescriptor);

  return this->save();
}

void Project::setCoreManagers(
  smtk::resource::ManagerPtr resManager,
  smtk::operation::ManagerPtr opManager)
{
  if (!m_resourceDescriptors.empty())
  {
    throw std::runtime_error("Cannot change core managers on open project");
  }

  if (!resManager)
  {
    throw std::runtime_error("Resource manager is null");
  }

  if (!opManager)
  {
    throw std::runtime_error("Operation manager is null");
  }

  resManager->registerResource<smtk::attribute::Resource>();
  m_resourceManager = resManager;
  m_operationManager = opManager;
}

bool Project::build(
  smtk::attribute::AttributePtr specification,
  smtk::io::Logger& logger,
  bool replaceExistingDirectory)
{
  // Get resource manager
  auto resManager = m_resourceManager.lock();
  if (!resManager)
  {
    smtkErrorMacro(logger, "Resource manager is null");
    return false;
  }

  // Note that specification is the "new-project" definition in NewProject.sbt.
  // Validate starting conditions
  if (!specification->isValid())
  {
    smtkErrorMacro(logger, "invalid project specification");
    return false;
  }

  // (Future) check that at least 1 resource specified?

  // Check for workspace folder
  std::string workspaceFolder = specification->findDirectory("workspace-path")->value(0);
  boost::filesystem::path workspacePath(workspaceFolder);
  if (!boost::filesystem::exists(workspacePath))
  {
    if (!boost::filesystem::create_directory(workspacePath))
    {
      smtkErrorMacro(logger, "Unable to create workspace folder \"" << workspaceFolder << "\"");
      return false;
    }
  }

  // Get project name and path
  m_name = specification->findString("project-folder")->value(0);
  auto projectPath = workspacePath / boost::filesystem::path(m_name);
  m_directory = projectPath.string();

  // Check if project directory already exists
  if (boost::filesystem::exists(projectPath) && !boost::filesystem::is_empty(projectPath))
  {
    if (replaceExistingDirectory)
    {
      boost::filesystem::remove_all(projectPath);
    }
    else
    {
      smtkErrorMacro(
        logger, "Cannot create project in existing directory: \"" << m_directory << "\"");
      return false;
    }
  } // if (projectPath exists)
  boost::filesystem::create_directory(projectPath);

  // Import model resources
  if (!this->importModels(specification, logger))
  {
    return false;
  }

  // Initialize modelDescriptor to the primary model.
  // At this point, only models have been loaded, and the default model is
  // loaded first.
  ResourceDescriptor modelDescriptor;
  if (!m_resourceDescriptors.empty())
  {
    modelDescriptor = m_resourceDescriptors[0];
  }

  // Import the attribute template
  ResourceDescriptor attDescriptor;
  auto attFileItem = specification->findFile("simulation-template");
  if (attFileItem->isEnabled())
  {
    std::string attFileValue = attFileItem->value(0);
    bool success = this->importAttributeTemplate(attFileValue, attDescriptor, logger);
    if (!success)
    {
      return false;
    }

    // Extract the name of the simulation code from attFilePath
    boost::filesystem::path attPath(attFileValue);
    auto simCode = attPath.stem().string();
    std::transform(simCode.begin(), simCode.end(), simCode.begin(), ::tolower);
    m_simulationCode = simCode;

    m_resourceDescriptors.push_back(attDescriptor);
  } // if (attFileItem enabled)

  // Link attribute resource to model resource
  if (!modelDescriptor.m_uuid.isNull() && !attDescriptor.m_uuid.isNull())
  {
    auto attResource = resManager->get(attDescriptor.m_uuid);
    auto modelResource = resManager->get(modelDescriptor.m_uuid);
    smtk::dynamic_pointer_cast<smtk::attribute::Resource>(attResource)->associate(modelResource);
  }

  // Write project files and return
  return this->save(logger);
}

bool Project::save(smtk::io::Logger& logger) const
{
  auto resManager = m_resourceManager.lock();
  if (!resManager)
  {
    smtkErrorMacro(logger, "Resource manager is null");
    return false;
  }

  auto opManager = m_operationManager.lock();
  if (!opManager)
  {
    smtkErrorMacro(logger, "Operation manager is null");
    return false;
  }

  boost::filesystem::path boostDirectory(m_directory);

  // Save project resources
  for (const auto& rd : m_resourceDescriptors)
  {
    auto resource = resManager->get(rd.m_uuid);
    // Check if resource is modified or attribute type. (Always save attribute
    // resources because some edits don't mark the resource as modified.)
    if (resource->clean() && !resource->isOfType<smtk::attribute::Resource>())
    {
      continue;
    }
    auto writer = opManager->create<smtk::operation::WriteResource>();
    writer->parameters()->associate(resource);
    auto result = writer->operate();
    int outcome = result->findInt("outcome")->value(0);
    if (outcome != SUCCEEDED)
    {
      smtkErrorMacro(
        logger, "Error writing resource file " << resource->location() << ", outcome: " << outcome);
      return false;
    }
  } // for (rd)

  return this->writeProjectFile(logger);
} // save()

bool Project::close()
{
  auto resManager = m_resourceManager.lock();
  if (!resManager)
  {
    return false;
  }

  this->releaseExportOperator();

  // Release resources
  for (const auto& rd : m_resourceDescriptors)
  {
    auto resourcePtr = resManager->get(rd.m_uuid);
    if (resourcePtr)
    {
      resManager->remove(resourcePtr);
    }
  }
  m_resourceDescriptors.clear();
  m_name.clear();
  m_directory.clear();

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

  m_simulationCode = descriptor.m_simulationCode;
  m_name = descriptor.m_name;
  m_directory = descriptor.m_directory;
  m_resourceDescriptors = descriptor.m_resourceDescriptors;

  return this->loadResources(directoryPath.string(), logger);
} // open()

bool Project::copyTo(ProjectPtr thatProject, smtk::io::Logger& logger) const
{
#ifndef NDEBUG
  std::cout << "new project name: " << thatProject->name()
            << "new project folder: " << thatProject->directory() << std::endl;
#endif

  thatProject->m_simulationCode = m_simulationCode;

  // Modify my project resources for the target project
  boost::filesystem::path thatProjectPath(thatProject->directory());
  for (const auto& rd : m_resourceDescriptors)
  {
    auto resManager = m_resourceManager.lock();
    if (resManager == nullptr)
    {
      smtkErrorMacro(logger, "resource manager is null");
      return false;
    }

    auto resource = resManager->get(rd.m_uuid);
    if (resource == nullptr)
    {
      smtkErrorMacro(logger, "Resource \"" << rd.m_filename << "\" is missing.");
      return false;
    }

    // Change id and location
    auto uuid = smtk::common::UUID::random();
    resource->setId(uuid);

    auto locationPath = thatProjectPath / rd.m_filename;
    resource->setLocation(locationPath.string());

    // Construct descriptor
    ResourceDescriptor thatRD;
    thatRD.m_filename = rd.m_filename;
    thatRD.m_identifier = rd.m_identifier;
    thatRD.m_importLocation = rd.m_importLocation;
    thatRD.m_typeName = rd.m_typeName;
    thatRD.m_uuid = uuid;

    // Special handling for models
    // If the import location is inside "this" project, copy the file to "that" project
    if (resource->isOfType<smtk::model::Resource>())
    {
      // Check if native model was copied into this project
      boost::filesystem::path thisProjectPath(m_directory);
      boost::filesystem::path thisImportPath(rd.m_importLocation);
      thisImportPath.remove_filename();
      if (thisProjectPath == thisImportPath)
      {
        // Copy the native model file to that project
        boost::filesystem::path thisImportFilePath(rd.m_importLocation);
        boost::filesystem::path thatImportFilePath(thatProject->m_directory);
        thatImportFilePath /= thisImportFilePath.filename();
        boost::system::error_code errcode;
        boost::filesystem::copy_file(
          thisImportFilePath, thatImportFilePath, boost::filesystem::copy_option::none, errcode);
        if (errcode)
        {
          smtkErrorMacro(logger, errcode.message());
          return false;
        }

        // Update url in resource's model
        auto modelResource = std::dynamic_pointer_cast<smtk::model::Resource>(resource);
        auto modelList = modelResource->findEntitiesOfType(smtk::model::MODEL_ENTITY, true);
        auto model = modelList.front();
        modelResource->setStringProperty(model.entity(), "url", thatImportFilePath.string());

        // Update import location in resource descriptor
        thatRD.m_importLocation = thatImportFilePath.string();
      } // if (copying native model)
    }   // if (model resource)

    thatProject->m_resourceDescriptors.push_back(thatRD);
  } // for (resource descriptors)

  return true;
} // copyTo()

bool Project::importModels(
  const smtk::attribute::AttributePtr specification,
  smtk::io::Logger& logger)
{
  bool useVTKSession = specification->findVoid("use-vtk-session")->isEnabled();
  auto modelGroupItem = specification->findGroup("model-group");
  bool copyFile = modelGroupItem->find("copy-file")->isEnabled();

  for (int i = 0; i < 2; ++i)
  {
    smtk::attribute::ItemPtr baseItem;
    std::string identifier;
    switch (i)
    {
      case 0:
        baseItem = modelGroupItem->find("model-file");
        identifier = "default";
        break;

      case 1:
        baseItem = modelGroupItem->find("second-model-file");
        identifier = "second";
        break;

      default:
        assert(false);
    } // switch
    assert(!!baseItem);

    if (baseItem->isEnabled())
    {
      auto fileItem = smtk::dynamic_pointer_cast<smtk::attribute::FileItem>(baseItem);
      std::string modelPath = fileItem->value(0);

      ResourceDescriptor modelDescriptor;
      if (!this->importModel(modelPath, copyFile, modelDescriptor, useVTKSession, logger))
      {
        return false;
      }
      modelDescriptor.m_identifier = identifier;
      m_resourceDescriptors.push_back(modelDescriptor);
    }
  } // for (i)

  return true;
}

bool Project::importModel(
  const std::string& importPath,
  bool copyNativeModel,
  ResourceDescriptor& descriptor,
  bool useVTKSession,
  smtk::io::Logger& logger)
{
  auto opManager = m_operationManager.lock();
  if (!opManager)
  {
    smtkErrorMacro(logger, "Operation manager is null");
    return false;
  }
  smtkDebugMacro(logger, "Loading model " << importPath);

  boost::filesystem::path boostImportPath(importPath);
  boost::filesystem::path boostDirectory(m_directory);

  // Copy the import (native) model file
  if (copyNativeModel)
  {
    auto copyPath = boostDirectory / boostImportPath.filename();
    boost::system::error_code errcode;
    boost::filesystem::copy_file(
      importPath, copyPath, boost::filesystem::copy_option::none, errcode);
    if (errcode)
    {
      smtkErrorMacro(logger, errcode.message());
      return false;
    }
    descriptor.m_importLocation = copyPath.string();

    // And update the import path to use the copied file
    boostImportPath = copyPath;
  }

  // Create the import operator
  smtk::operation::OperationPtr importOp;
  if (useVTKSession)
  {
    importOp = opManager->create("smtk::session::vtk::Import");
  }
  else
  {
    importOp = opManager->create<smtk::operation::ImportResource>();
  }

  if (!importOp)
  {
    smtkErrorMacro(logger, "Import operator not found");
    return false;
  }

  int outcome;

  // Run the import operator
  std::string useImportPath = boostImportPath.string();
  importOp->parameters()->findFile("filename")->setValue(useImportPath);
  auto importOpResult = importOp->operate();
  outcome = importOpResult->findInt("outcome")->value(0);
  if (outcome != SUCCEEDED)
  {
    smtkErrorMacro(logger, "Error importing file " << useImportPath);
    return false;
  }
  auto modelResource = importOpResult->findResource("resource")->value(0);

  // Set location to project directory
  auto modelFilename = boostImportPath.filename().string() + ".smtk";
  auto smtkPath = boostDirectory / boost::filesystem::path(modelFilename);
  modelResource->setLocation(smtkPath.string());

  // Update the descriptor
  descriptor.m_filename = modelFilename;
  descriptor.m_identifier = modelResource->name();
  descriptor.m_importLocation = useImportPath;
  descriptor.m_typeName = modelResource->typeName();
  descriptor.m_uuid = modelResource->id();

  return true; // success
} // importModel()

bool Project::importAttributeTemplate(
  const std::string& location,
  ResourceDescriptor& descriptor,
  smtk::io::Logger& logger)
{
  auto resManager = m_resourceManager.lock();
  if (!resManager)
  {
    smtkErrorMacro(logger, "Resource manager is null");
    return false;
  }

  smtkDebugMacro(logger, "Loading templateFile: " << location);

  // Read from specified location
  auto attResource = smtk::attribute::Resource::create();
  smtk::io::AttributeReader reader;
  bool readErr = reader.read(attResource, location, true, logger);
  if (readErr)
  {
    return false; // invert from representing "error" to representing "success"
  }
  resManager->add(attResource);

  // Update descriptor
  if (descriptor.m_identifier.empty())
  {
    descriptor.m_identifier = "default";
  }
  descriptor.m_filename = std::string("sbi.") + descriptor.m_identifier + ".smtk";
  descriptor.m_importLocation = location;
  descriptor.m_typeName = attResource->typeName();
  descriptor.m_uuid = attResource->id();

  boost::filesystem::path boostDirectory(m_directory);
  boost::filesystem::path resourcePath =
    boostDirectory / boost::filesystem::path(descriptor.m_filename);
  attResource->setLocation(resourcePath.string());
  return true;
} // importAttributeTemplate()

bool Project::writeProjectFile(smtk::io::Logger& logger) const
{
  // Init ProjectDescriptor structure
  ProjectDescriptor descriptor;
  descriptor.m_simulationCode = m_simulationCode;
  descriptor.m_name = m_name;
  descriptor.m_directory = m_directory;
  descriptor.m_resourceDescriptors = m_resourceDescriptors;

  // Get json string
  std::string dotFileContents =
    dump_json(descriptor); // (static function in jsonProjectDescriptor.h)

  // Write file
  std::ofstream projectFile;
  auto path = boost::filesystem::path(m_directory) / boost::filesystem::path(PROJECT_FILENAME);
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
  auto opManager = m_operationManager.lock();
  if (!opManager)
  {
    smtkErrorMacro(logger, "Operation manager is null");
    return false;
  }

  // Part of opening project from disk
  boost::filesystem::path directoryPath(path);

  for (const auto& descriptor : m_resourceDescriptors)
  {
    // Create a read operator
    auto readOp = opManager->create<smtk::operation::ReadResource>();
    if (!readOp)
    {
      smtkErrorMacro(logger, "Read Resource operator not found");
      return false;
    }

    auto filePath = directoryPath / boost::filesystem::path(descriptor.m_filename);
    readOp->parameters()->findFile("filename")->setValue(filePath.string());
    auto readOpResult = readOp->operate();

    // Check outcome
    int outcome = readOpResult->findInt("outcome")->value(0);
    if (outcome != SUCCEEDED)
    {
      smtkErrorMacro(logger, "Error loading resource from: " << filePath.string());
      return false;
    }
  } // for (descriptor)

  return true;
} // loadResources()

smtk::operation::OperationPtr Project::getExportOperator(smtk::io::Logger& logger, bool reset)
{
#ifndef SMTK_PYTHON_ENABLED
  (void)reset;
  smtkErrorMacro(logger, "Python export operators are not supported in this SMTK build");
  return smtk::operation::OperationPtr();
#else
  auto opManager = m_operationManager.lock();
  if (!opManager)
  {
    smtkErrorMacro(logger, "Operation manager is null");
    return smtk::operation::OperationPtr();
  }

  // Check if already loaded
  if (!!m_exportOperator)
  {
    if (reset)
    {
      this->releaseExportOperator();
    }
    else
    {
      return m_exportOperator;
    }
  }

  // For now, find export operator based on fixed relative path from simulation template
  // to the sbt import directory.
  // Future: add .smtk info file to workflow directories

  // Find the simulation attribute resource.
  ResourceDescriptor simAttDescriptor;
  for (const auto& descriptor : m_resourceDescriptors)
  {
    if (descriptor.m_typeName == smtk::common::typeName<smtk::attribute::Resource>())
    {
      simAttDescriptor = descriptor;
      break;
    }
  } // for

  if (simAttDescriptor.m_filename.empty())
  {
    smtkErrorMacro(logger, "simulation attribute not found, so no export operator defined");
    return nullptr;
  }

  if (simAttDescriptor.m_importLocation.empty())
  {
    smtkErrorMacro(
      logger, "simulation resource missing import location - cannot find export operator");
    return nullptr;
  }

  // Copy the import location and change extension from .sbt to .py
  std::string location(simAttDescriptor.m_importLocation);
  std::string key(".sbt");
  auto pos = location.rfind(key);
  if (pos == std::string::npos)
  {
    smtkErrorMacro(logger, "import location (" << location << ") does not end in .sbt");
    return nullptr;
  }

  location.replace(pos, key.length(), ".py");
  boost::filesystem::path locationPath(location);

  if (!boost::filesystem::exists(locationPath))
  {
    smtkErrorMacro(logger, "Could not find export operator file " << location);
    return nullptr;
  }

  smtk::operation::OperationPtr importPythonOp =
    opManager->create<smtk::operation::ImportPythonOperation>();
  if (!importPythonOp)
  {
    smtkErrorMacro(logger, "Could not create \"import python operation\"");
    return nullptr;
  }

  // Set the input python operation file name
  importPythonOp->parameters()->findFile("filename")->setValue(location);

  smtk::operation::Operation::Result result;
  try
  {
    // Execute the operation
    result = importPythonOp->operate();
  }
  catch (std::exception& e)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), e.what());
    return nullptr;
  }

  // Test the results for success
  int outcome = result->findInt("outcome")->value();
  if (outcome != static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "\"import python operation\" operation failed, outcome " << outcome);
    return nullptr;
  }

  // On success, the ImportPythonOperation creates a "unique_name" value.
  // Use that string to create the export operator, and save that string to (later) release
  // the export operator.
  m_exportOperatorUniqueName = result->findString("unique_name")->value();
  m_exportOperator = opManager->create(m_exportOperatorUniqueName);
  this->populateExportOperator(m_exportOperator, logger);

  return m_exportOperator;

#endif // PYTHON_ENABLED
}

bool Project::populateExportOperator(
  smtk::operation::OperationPtr exportOp,
  smtk::io::Logger& /*logger*/) const
{
  // Locate project attribute and model resources
  std::vector<smtk::resource::ResourcePtr> attResourceList;
  std::vector<smtk::resource::ComponentPtr> modelList;
  auto resourceList = this->resources();
  for (const auto& resource : resourceList)
  {
    if (resource->isOfType(smtk::common::typeName<smtk::attribute::Resource>()))
    {
      attResourceList.push_back(resource);
    }
    else if (resource->isOfType(smtk::common::typeName<smtk::model::Resource>()))
    {
      auto modelResource = smtk::dynamic_pointer_cast<smtk::model::Resource>(resource);
      auto uuids = modelResource->entitiesMatchingFlags(smtk::model::MODEL_ENTITY, true);
      for (const auto& uuid : uuids)
      {
        auto model = modelResource->find(uuid);
        if (model)
        {
          modelList.push_back(model);
        }
      }
    }
  } // for (resource)

  // Check parameters for "attributes" and "model" items
  auto paramAttribute = exportOp->parameters();

  auto attItem = paramAttribute->findResource("attributes");
  if (attItem)
  {
    if (attResourceList.size() == 1)
    {
      attItem->setValue(attResourceList[0]);
    }
  }

  auto modelItem = paramAttribute->findComponent("model");
  if (modelItem)
  {
    if (modelList.size() == 1)
    {
      modelItem->setValue(modelList[0]);
    }
  }

  // If there is a single DirectoryItem, set it to a "sim" folder below the project
  std::vector<smtk::attribute::ItemPtr> itemList;
  int numItems = static_cast<int>(paramAttribute->numberOfItems());
  for (int i = 0; i < numItems; ++i)
  {
    auto item = paramAttribute->item(i);
    if (item->type() == smtk::attribute::Item::DirectoryType)
    {
      itemList.push_back(item);
    }
  } // for
  if (itemList.size() == 1)
  {
    auto dirItem = dynamic_pointer_cast<smtk::attribute::DirectoryItem>(itemList[0]);
    auto boostPath = boost::filesystem::path(m_directory) / boost::filesystem::path("sim");
    dirItem->setValue(boostPath.string());
  }

  return true;
}

void Project::releaseExportOperator()
{
  if (!m_exportOperator)
  {
    return;
  }

  auto opManager = m_operationManager.lock();
  if (!opManager)
  {
    return;
  }

  opManager->unregisterOperation(m_exportOperatorUniqueName);
  m_exportOperator = nullptr;
  m_exportOperatorUniqueName.clear();
}

} // namespace project
} // namespace smtk
