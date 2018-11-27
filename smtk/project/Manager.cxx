//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/project/Manager.h"
#include "smtk/project/NewProjectTemplate.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
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
#include "smtk/model/Resource.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"

#include "boost/filesystem.hpp"

#include <exception>
#include <fstream>
#include <tuple>

namespace
{
int FAILED = static_cast<int>(smtk::operation::Operation::Outcome::FAILED);       // 2
int SUCCEEDED = static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED); // 3
}

namespace smtk
{
namespace project
{
Manager::Manager()
{
}

Manager::~Manager()
{
}

void Manager::setManagers(
  smtk::resource::ManagerPtr& resourceManager, smtk::operation::ManagerPtr& operationManager)
{
  if (!this->m_resourceInfos.empty())
  {
    throw std::runtime_error("Cannot set internal managers with project open");
  }
  this->m_resourceManager = resourceManager;
  this->m_resourceManager->registerResource<smtk::attribute::Resource>();
  this->m_operationManager = operationManager;
}

smtk::attribute::ResourcePtr Manager::getProjectTemplate() const
{
  auto reader = smtk::io::AttributeReader();
  auto newTemplate = smtk::attribute::Resource::create();
  auto logger = smtk::io::Logger();
  reader.readContents(newTemplate, NewProjectTemplate, logger);
  return newTemplate;
}

smtk::attribute::AttributePtr Manager::getProjectSpecification() const
{
  auto newTemplate = this->getProjectTemplate();
  std::string name = "new-project";
  auto defn = newTemplate->findDefinition(name);
  // std::cout << "defn:" << defn << std::endl;
  auto att = newTemplate->createAttribute(name, defn);
  return att;
}

int Manager::createProject(smtk::attribute::AttributePtr specification)
{
  int outcome = static_cast<int>(smtk::operation::Operation::Outcome::UNKNOWN); // return value

  if (!this->m_resourceManager)
  {
    throw std::runtime_error("Cannot create project - must set resource manager first");
  }
  if (!this->m_operationManager)
  {
    throw std::runtime_error("Cannot create project - must set operation manager first");
  }

  // Todo check that at least 1 resource specified?

  // Initialize project directory
  auto directory = specification->findDirectory("project-directory")->value(0);
  boost::filesystem::path boostDirectory(directory);
  if (boost::filesystem::exists(boostDirectory))
  {
    boost::filesystem::remove_all(boostDirectory);
  }
  boost::filesystem::create_directory(boostDirectory);

  // Load the model resource
  auto modelFileItem = specification->findFile("model-file");
  std::string modelPath;
  smtk::model::ResourcePtr modelResource;
  ResourceInfo modelInfo;
  if (modelFileItem->isEnabled())
  {
    modelPath = modelFileItem->value(0);
    // Check option to copy the import file
    if (specification->findVoid("copy-model-file")->isEnabled())
    {
      boost::filesystem::path boostModelPath(modelPath);
      auto copyPath = boostDirectory / boostModelPath.filename();
      boost::filesystem::copy_file(modelPath, copyPath);
      modelPath = copyPath.string();
    }

    std::cerr << "Load model " << modelPath << std::endl;
    auto tuple = this->importModel(modelPath);
    //std::tie(outcome, modelResource) = tuple;
    outcome = std::get<0>(tuple);
    smtk::resource::ResourcePtr r = std::get<1>(tuple);
    modelResource = smtk::dynamic_pointer_cast<smtk::model::Resource>(r);
    if (outcome != SUCCEEDED)
    {
      return outcome;
    }

    // Initialize ResourceInfo
    modelInfo.m_identifier = "model";
    modelInfo.m_importLocation = modelPath;
    modelInfo.m_role = "default";
    modelInfo.m_typeName = modelResource->typeName();
    modelInfo.m_uuid = modelResource->id();
  } // if (modelFileItem enabled)

  // Load the (simulation) attribute resource
  smtk::attribute::ResourcePtr attResource;
  auto templateFileItem = specification->findFile("simulation-template");
  ResourceInfo attInfo;
  if (templateFileItem->isEnabled())
  {
    auto templateFile = templateFileItem->value(0);
    std::cerr << "Load templateFile: " << templateFile << std::endl;
    attResource = smtk::attribute::Resource::create();
    smtk::io::AttributeReader reader;
    smtk::io::Logger logger;
    bool err = reader.read(attResource, templateFile, true, logger);
    if (err)
    {
      std::cerr << logger.convertToString(true) << std::endl;
      attResource.reset();
      return FAILED;
    }
    outcome = SUCCEEDED;
    this->m_resourceManager->add(attResource);

    // Initialize ResourceInfo
    attInfo.m_identifier = "default";
    attInfo.m_importLocation = templateFile;
    attInfo.m_role = "default";
    attInfo.m_typeName = attResource->typeName();
    attInfo.m_uuid = attResource->id();
  }

  // Create directory and save resources
  // Save model resource
  if (modelResource)
  {
    auto modelWriter = this->m_operationManager->create("smtk::operation::WriteResource");
    modelWriter->parameters()->associate(modelResource);
    modelWriter->parameters()->find("filename")->setIsEnabled(true);

    boost::filesystem::path boostModelPath(modelPath);
    auto modelFilename = boostModelPath.stem();
    auto smtkFilename = modelFilename.replace_extension("smtk");
    auto smtkPath = boostDirectory / smtkFilename;
    modelWriter->parameters()->findFile("filename")->setValue(0, smtkPath.string());

    auto result = modelWriter->operate();
    auto outcome = result->findInt("outcome")->value(0);
    if (outcome != SUCCEEDED)
    {
      return outcome;
    }
    modelInfo.m_filename = smtkFilename.string();
  } // if (modelResource)

  // Save attribute resource
  if (attResource)
  {
    attInfo.m_filename = "default.sbi";
    auto writer = smtk::io::AttributeWriter();
    boost::filesystem::path sbiPath = boostDirectory / boost::filesystem::path(attInfo.m_filename);
    auto logger = smtk::io::Logger();
    bool err = writer.write(attResource, sbiPath.string(), logger);
    if (err)
    {
      attResource.reset();
      if (modelResource)
      {
        modelResource.reset();
      }
      return FAILED;
    }
  } // if (attResource)

  // Update member data
  m_projectDirectory = directory;
  m_projectName = specification->findString("project-name")->value(0);
  if (modelResource)
  {
    m_resourceInfos.push_back(modelInfo);
  }
  if (attResource)
  {
    m_resourceInfos.push_back(attInfo);
  }

  outcome = this->writeProjectFile();
  return outcome;
}

smtk::resource::ResourcePtr Manager::getResourceByRole(
  const std::string& typeName, const std::string& role) const
{
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

std::tuple<bool, std::string, std::string> Manager::getStatus() const
{
  bool isProject = !this->m_resourceInfos.empty();
  std::tuple<bool, std::string, std::string> retval =
    std::make_tuple(isProject, this->m_projectName, this->m_projectDirectory);
  return retval;
}

std::vector<smtk::project::ResourceInfo> Manager::getResourceInfos() const
{
  return std::vector<smtk::project::ResourceInfo>();
}

int Manager::saveProject()
{
  boost::filesystem::path boostDirectory(this->m_projectDirectory);
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
}

int Manager::closeProject()
{
  // Todo check for modified resources ?

  // Release resources
  for (auto& resourceInfo : this->m_resourceInfos)
  {
    auto resourcePtr = this->m_resourceManager->get(resourceInfo.m_uuid);
    this->m_resourceManager->remove(resourcePtr);
    resourcePtr.reset();
  }
  this->m_resourceInfos.clear();
  this->m_projectName.clear();
  this->m_projectDirectory.clear();

  return SUCCEEDED;
}

int Manager::openProject(const std::string& projectPath)
{
  if (!this->m_resourceManager)
  {
    throw std::runtime_error("Cannot create project - must set resource manager first");
  }
  if (!this->m_operationManager)
  {
    throw std::runtime_error("Cannot create project - must set operation manager first");
  }

  if (!this->m_resourceInfos.empty())
  {
    std::cerr << "Must close current project before opening another project" << std::endl;
    return FAILED;
  }

  boost::filesystem::path directoryPath;
  boost::filesystem::path dotfilePath;

  boost::filesystem::path inputPath(projectPath);
  if (!boost::filesystem::exists(inputPath))
  {
    std::cerr << "Specified project path not found" << std::endl;
    return FAILED;
  }

  if (boost::filesystem::is_directory(inputPath))
  {
    directoryPath = inputPath;
    dotfilePath = directoryPath / boost::filesystem::path(".cmbproject");
    if (!boost::filesystem::exists(dotfilePath))
    {
      std::cerr << "No \".cmbproject\" file in specified directory" << std::endl;
      return FAILED;
    }
  }
  else if (inputPath.filename().string() != ".cmbproject")
  {
    std::cerr << "Invalid project filename, should be \".cmbproject\"" << std::endl;
    return FAILED;
  }
  else
  {
    dotfilePath = inputPath;
    directoryPath = inputPath.parent_path();
  }

  // Load .cmbproject file
  std::ifstream dotFile;
  dotFile.open(dotfilePath.string().c_str(), std::ios_base::in | std::ios_base::ate);
  if (!dotFile)
  {
    std::cerr << "Failed loading \".cmbproject\" file" << std::endl;
    return FAILED;
  }
  auto fileSize = dotFile.tellg();
  std::string dotFileContents;
  dotFileContents.reserve(fileSize);

  dotFile.seekg(0, std::ios_base::beg);
  dotFileContents.assign(
    (std::istreambuf_iterator<char>(dotFile)), std::istreambuf_iterator<char>());

  try
  {
    auto j = json::parse(dotFileContents);
    this->m_projectName = j.at("projectName");
    this->m_projectDirectory = j.at("projectDirectory");
    auto jInfos = j.at("resourceInfos");
    for (auto& jInfo : jInfos)
    {
      ::smtk::project::ResourceInfo info = jInfo;
      this->m_resourceInfos.push_back(info);
    }
  }
  catch (std::exception /* e */)
  {
    std::cerr << "Error loading \".cmbproject\" file" << std::endl;
    return FAILED;
  }

  // Load resources
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
}

smtk::attribute::ResourcePtr Manager::getExportTemplate() const
{
  return smtk::attribute::ResourcePtr();
}

int Manager::exportProject(smtk::attribute::ResourcePtr specification)
{
  return -1;
}

std::tuple<int, smtk::resource::ResourcePtr> Manager::importModel(const std::string& path)
{
  // Create an import operator
  auto importOp = this->m_operationManager->create("smtk::operation::ImportResource");
  if (!importOp)
  {
    throw std::runtime_error("No import operator");
  }
  importOp->parameters()->findFile("filename")->setValue(path);
  auto importOpResult = importOp->operate();

  // Test for success
  int outcome = importOpResult->findInt("outcome")->value(0);
  auto resource = importOpResult->findResource("resource")->value(0);
  return std::make_tuple(outcome, resource);
}

int Manager::writeProjectFile() const
{
  json j = {
    { "projectName", m_projectName }, { "projectDirectory", m_projectDirectory },
  };
  json jInfos = json::array();
  for (auto& resourceInfo : this->m_resourceInfos)
  {
    nlohmann::json jInfo = resourceInfo;
    jInfos.push_back(jInfo);
  }
  j["resourceInfos"] = jInfos;

  std::ofstream projectFile;
  auto path = boost::filesystem::path(m_projectDirectory) / boost::filesystem::path(".cmbproject");
  projectFile.open(path.string().c_str(), std::ofstream::out | std::ofstream::trunc);
  if (!projectFile)
  {
    return FAILED;
  }
  projectFile << j.dump(2) << "\n";
  projectFile.close();

  return SUCCEEDED;
}

} // namespace project
} // namespace smtk
