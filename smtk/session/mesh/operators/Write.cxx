//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/mesh/operators/Write.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/common/Archive.h"
#include "smtk/common/CompilerInformation.h"
#include "smtk/common/Paths.h"

#include "smtk/session/mesh/Resource.h"
#include "smtk/session/mesh/operators/Export.h"
#include "smtk/session/mesh/operators/Write_xml.h"

#include "smtk/model/json/jsonResource.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "nlohmann/json.hpp"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
SMTK_THIRDPARTY_POST_INCLUDE

#include <fstream>

using namespace smtk::model;

namespace smtk
{
namespace session
{

namespace mesh
{

bool Write::ableToOperate()
{
  if (!this->smtk::operation::XMLOperation::ableToOperate())
  {
    return false;
  }

  if (this->parameters()->associations()->numberOfValues() < 1)
  {
    return false;
  }

  return true;
}

Write::Result Write::operateInternal()
{
  auto resourceItem = this->parameters()->associations();

  smtk::session::mesh::Resource::Ptr resource =
    std::dynamic_pointer_cast<smtk::session::mesh::Resource>(resourceItem->value());

  // Serialize resource into a set of JSON records:
  nlohmann::json j = resource;
  if (j.is_null())
  {
    smtkErrorMacro(log(), "Unable to serialize model to json object.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Check if the mesh files are to be archived into a single file
  auto archive = this->parameters()->findVoid("archive");
  if (archive && archive->isEnabled())
  {
    boost::filesystem::path smtkFilename("index.json");
    boost::filesystem::path meshFilename("mesh.h5m");

    boost::filesystem::path temp =
      boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
    if (!boost::filesystem::create_directories(temp))
    {
      smtkErrorMacro(this->log(), "Failed to create a temporary directory.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    boost::filesystem::path tmpsmtkPath = temp / smtkFilename;
    boost::filesystem::path tmpMeshPath = temp / meshFilename;

    j["Mesh URL"] = meshFilename.string();

    // Write the smtk index
    {
      std::ofstream file(tmpsmtkPath.string());
      if (!file.good())
      {
        smtkErrorMacro(log(), "Unable to open \"" << tmpsmtkPath << "\" for writing.");
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
      file << j.dump(2);
      file.close();
    }

    // Write the mesh file
    {
      // Create a write operator
      smtk::session::mesh::Export::Ptr exportOp = smtk::session::mesh::Export::create();
      exportOp->parameters()->findFile("filename")->setValue(tmpMeshPath.string());

      // Set the entity association
      exportOp->parameters()->associate(resource);

      // Execute the operation
      smtk::operation::Operation::Result exportOpResult = exportOp->operate(Key());

      // Test for success
      if (
        exportOpResult->findInt("outcome")->value() !=
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
      {
        smtkErrorMacro(log(), "Unable to write mesh to  \"" << tmpMeshPath << "\".");
        ::boost::filesystem::remove_all(temp);
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
    }

    // Populate an archive with the smtk index and mesh file
    {
      smtk::common::Archive archive(resource->location());
      archive.insert(tmpsmtkPath.string(), smtkFilename.string());
      archive.insert(tmpMeshPath.string(), meshFilename.string());
      if (!archive.archive())
      {
        smtkErrorMacro(log(), "Unable to archive files to \"" + resource->location() + "\".");
        ::boost::filesystem::remove_all(temp);
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
    }

    // Clean up temporary files.
    {
      ::boost::filesystem::remove_all(temp);
    }

    return this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  }
  else
  {
    // Serialize resource into a set of JSON records:
    nlohmann::json j = resource;
    if (j.is_null())
    {
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    std::string meshFilename = resource->resource()->location();

    if (meshFilename.empty())
    {
      meshFilename = smtk::common::Paths::directory(resource->location()) + "/" +
        smtk::common::Paths::stem(resource->location()) + ".h5m";
    }

    j["Mesh URL"] = meshFilename;

    {
      std::ofstream file(resource->location());
      if (!file.good())
      {
        smtkErrorMacro(log(), "Unable to open \"" << resource->location() << "\" for writing.");
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
      file << j.dump(2);
      file.close();
    }

    // Create an export operator
    smtk::session::mesh::Export::Ptr exportOp = smtk::session::mesh::Export::create();
    exportOp->parameters()->findFile("filename")->setValue(meshFilename);

    // Set the entity association
    exportOp->parameters()->associate(resource);

    // Execute the operation
    smtk::operation::Operation::Result exportOpResult = exportOp->operate(Key());

    // Test for success
    if (
      exportOpResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      smtkErrorMacro(log(), "Unable to write files to \"" + meshFilename + "\".");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    // Add the mesh file to the result's list of additional files
    auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
    result->findFile("additional files")->appendValue(meshFilename);

    return result;
  }
}

const char* Write::xmlDescription() const
{
  return Write_xml;
}

void Write::markModifiedResources(Write::Result& /*unused*/)
{
  auto resourceItem = this->parameters()->associations();
  for (auto rit = resourceItem->begin(); rit != resourceItem->end(); ++rit)
  {
    auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(*rit);

    // Set the resource as unmodified from its persistent (i.e. on-disk) state
    resource->setClean(true);
  }
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

} // namespace mesh
} // namespace session
} // namespace smtk
