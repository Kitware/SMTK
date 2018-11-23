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

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

// Provides NewProjectTemplate string
#include "smtk/project/NewProjectTemplate.h"

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

void Manager::setResourceManager(smtk::resource::ManagerPtr&)
{
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
  return -1;
}

std::tuple<bool, std::string, std::string> Manager::getStatus() const
{
  std::tuple<bool, std::string, std::string> retval = std::make_tuple(false, "", "");
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
  return -1;
}

int Manager::loadProject(const std::string& path)
{
  return -1;
}

smtk::attribute::ResourcePtr Manager::getExportTemplate() const
{
  return nullptr;
}

int Manager::exportProject(smtk::attribute::ResourcePtr specification)
{
  return -1;
}

} // namespace project
} // namespace smtk
