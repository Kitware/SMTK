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

  if (!m_resourceManager)
  {
    throw std::runtime_error("Cannot create project - must set resource manager first");
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
    //auto attResource = this->m_resourceManager->create<smtk::attribute::Resource>();
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
  return -1;
}

int Manager::closeProject()
{
  // Todo check for modified resources ?

  // Release resources
  for (auto& resourceInfo : this->m_resourceInfos)
  {
    auto resourcePtr = this->m_resourceManager->get(resourceInfo.m_uuid);
    resourcePtr.reset();
  }
  this->m_resourceInfos.clear();
  this->m_projectName.clear();
  this->m_projectDirectory.clear();

  return SUCCEEDED;
}

int Manager::loadProject(const std::string& path)
{
  return -1;
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
  // if (outcome != SUCCEEDED)
  // {
  //   throw std::runtime_error("Import operator failed");
  // }
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
    nlohmann::json jInfo = resourceInfo.to_json();
    jInfos.push_back(jInfo);
  }
  j["resourceInfos"] = jInfos;
  //std::cout << j.dump(2) << std::endl;

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
