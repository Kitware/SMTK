//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/mesh/operators/Read.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/common/Archive.h"
#include "smtk/common/CompilerInformation.h"

#include "smtk/model/json/jsonResource.h"

#include "smtk/session/mesh/Resource.h"

#include "smtk/session/mesh/operators/Import.h"
#include "smtk/session/mesh/operators/Read_xml.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "nlohmann/json.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <fstream>

namespace smtk
{
namespace session
{
namespace mesh
{

Read::Result Read::operateInternal()
{
  std::string filename = this->parameters()->findFile("filename")->value();

  std::ifstream file;

  smtk::common::Archive archive(filename);
  if (!archive.contents().empty())
  {
    std::string smtkFilename = "index.json";

    archive.get(smtkFilename, file);
  }
  else
  {
    file.open(filename);
  }

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

  // Access the resource's id
  std::string resourceIdStr = j.at("id");
  smtk::common::UUID resourceId(resourceIdStr);

  // Create a new resource for the import
  auto resource = smtk::session::mesh::Resource::create();
  resource->setId(resourceId);
  auto session = smtk::session::mesh::Session::create();
  resource->setLocation(filename);
  resource->setSession(session);

  // Transcribe model data onto the resource
  auto modelResource = std::static_pointer_cast<smtk::model::Resource>(resource);
  smtk::model::from_json(j, modelResource);

  std::string meshFilename = j.at("Mesh URL");

  if (!archive.contents().empty())
  {
    meshFilename = archive.location(meshFilename);
  }

  // Create an import operator
  smtk::session::mesh::Import::Ptr importOp = smtk::session::mesh::Import::create();
  importOp->callFromRead = true;
  importOp->parameters()->associate(resource);
  importOp->parameters()->findString("session only")->setDiscreteIndex(0);
  importOp->parameters()->findFile("filename")->setValue(meshFilename);
  importOp->parameters()->findVoid("construct hierarchy")->setIsEnabled(false);

  // Execute the operation
  smtk::operation::Operation::Result importOpResult = importOp->operate();

  // Retrieve the resulting resource
  smtk::attribute::ResourceItemPtr resourceItem =
    std::dynamic_pointer_cast<smtk::attribute::ResourceItem>(
      importOpResult->findResource("resourcesCreated"));

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resourcesCreated");
    created->appendValue(resourceItem->value());
  }

  return result;
}

const char* Read::xmlDescription() const
{
  return Read_xml;
}

void Read::markModifiedResources(Read::Result& res)
{
  auto resourceItem = res->findResource("resourcesCreated");
  for (std::size_t ii = 0; ii < resourceItem->numberOfValues(); ++ii)
  {
    if (resourceItem->isSet(ii))
    {
      auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(resourceItem->value(ii));

      // Set the resource as unmodified from its persistent (i.e. on-disk) state
      resource->setClean(true);
    }
  }
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
  return result->findResource("resourcesCreated")->value();
}
} // namespace mesh
} // namespace session
} // namespace smtk
