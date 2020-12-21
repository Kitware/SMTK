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

#include "smtk/common/Archive.h"
#include "smtk/common/Paths.h"

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

namespace
{
void updateFileReferencesInString(
  std::string& fileContents,
  const std::map<std::string, std::string>& fileDictionary)
{
  auto findAndReplace =
    [](std::string& file_contents, const std::string& from, const std::string& to) {
      auto pos = file_contents.find(from);
      while (pos != std::string::npos)
      {
        file_contents.replace(pos, from.length(), to);
        pos += to.length();
        pos = file_contents.find(from, pos);
      }
    };

  for (const auto& path : fileDictionary)
  {
    findAndReplace(fileContents, path.first, path.second);
  }
}

void updateFileReferencesInFile(
  const std::string& file,
  const std::map<std::string, std::string>& fileDictionary)
{
  std::ifstream t(file);
  std::string fileContents;
  t.seekg(0, std::ios::end);
  fileContents.reserve(t.tellg());
  t.seekg(0, std::ios::beg);

  fileContents.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

  updateFileReferencesInString(fileContents, fileDictionary);

  {
    std::ofstream out(file);
    out << fileContents;
    out.close();
  }
}
} // namespace

namespace smtk
{
namespace project
{

Write::Result Write::operateInternal()
{
  // Access the project to write.
  smtk::attribute::ReferenceItem::Ptr projectItem = this->parameters()->associations();
  smtk::project::ProjectPtr project = projectItem->valueAs<smtk::project::Project>();

  // Access the file name to write.
  std::string outputFile = project->location();

  // Construct an archive using the output file name.
  smtk::common::Archive archive(outputFile);

  // Construct a temporary location to hold resources that are not already on
  // disk and the additional json file described by the project.
  boost::filesystem::path temp =
    boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  if (!boost::filesystem::create_directories(temp))
  {
    smtkErrorMacro(this->log(), "Failed to create a temporary directory.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Construct a WriteResource operation to write all of the project's
  // resources.
  smtk::operation::WriteResource::Ptr write =
    project->operations().manager()->create<smtk::operation::WriteResource>();
  if (!write)
  {
    smtkErrorMacro(this->log(), "Cannot create WriteResource operation.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  std::set<smtk::resource::Resource*> unassignedResources;
  std::map<std::string, std::string> resourceDictionary;

  // Insert the project's resources into the archive.
  for (auto& resource : project->resources())
  {
    // Reset the write operation's associations.
    write->parameters()->associations()->reset();

    // Access the resource's role
    const std::string& role = detail::role(resource);

    // If the resource is not clean, we must write it to disk before we can
    // add it to the archive.
    if (!resource->clean())
    {
      if (resource->location().empty())
      {
        unassignedResources.insert(resource.get());
        resource->setLocation((temp / boost::filesystem::path(role + ".smtk")).string());
      }

      write->parameters()->associate(resource);
      smtk::operation::Operation::Result writeResult = write->operate();
      if (
        writeResult->findInt("outcome")->value() !=
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
      {
        // An error message should already enter the logger from the local
        // operation.
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }

      // Some resource writers write support files in addition to the primary
      // .smtk file. When this happens, we rely on those operations to report
      // their additional files in the operation's result.
      auto writtenFiles = writeResult->findFile("files");
      if (writtenFiles->numberOfValues() > 1)
      {
        // A map between a file's path and its lookup in the archive.
        std::map<std::string, std::string> fileDictionary;

        for (int i = 0; i < writtenFiles->numberOfValues(); ++i)
        {
          std::string fileName = writtenFiles->value(i);

          // If the file is the resource file, we don't have to do anything
          // special.
          if (fileName == resource->location())
          {
            continue;
          }

          // Otherwise, store the file's path and its archive path.
          fileDictionary[fileName] = role + "/" + smtk::common::Paths::filename(fileName);
        }

        // Insert the ancillary files into the archive.
        for (const auto& path : fileDictionary)
        {
          // This fixes error writing .exo file, but not sure it addresses the
          // underlying problem (john).
          std::string tempPath = (temp / path.first).string();
          archive.insert(tempPath, path.second);
        }

        // Update the resource's json file to use the archive file locations
        updateFileReferencesInFile(resource->location(), fileDictionary);
      }
    }

    resourceDictionary[resource->location()] = role + ".smtk";
    archive.insert(resource->location(), role + ".smtk");
  }

  // All resources have been written do disk at locations in resourceDictionary.
  // We now write the project's json file and update its file paths.
  {
    nlohmann::json j = project;
    if (j.is_null())
    {
      smtkErrorMacro(log(), "Unable to serialize project to json object.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    std::string fileContents = j.dump(2);

    updateFileReferencesInString(fileContents, resourceDictionary);

    std::string indexFileName = (temp / boost::filesystem::path("index.json")).string();
    std::ofstream file(indexFileName);
    file << fileContents;
    file.close();

    archive.insert(indexFileName, "index.json");
  }

  // Write the archive to disk.
  bool success = archive.archive();
  if (!success)
  {
    smtkErrorMacro(log(), "Failed to archive project contents.");
  }

  // Construct a result object.
  auto result =
    (success ? this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED)
             : this->createResult(smtk::operation::Operation::Outcome::FAILED));

  // Unassign any temporary locations we assigned.
  if (!unassignedResources.empty())
  {
    for (auto resource : unassignedResources)
    {
      resource->setLocation("");
    }
  }

  // Remove the temporary directory we created.
  ::boost::filesystem::remove_all(temp);

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
