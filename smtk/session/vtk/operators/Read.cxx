//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/vtk/operators/Read.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/session/vtk/Resource.h"
#include "smtk/session/vtk/json/jsonResource.h"
#include "smtk/session/vtk/operators/Import.h"
#include "smtk/session/vtk/operators/Read_xml.h"

#include "smtk/common/Paths.h"

#include "smtk/model/SessionIOJSON.h"

using namespace smtk::model;

namespace smtk
{
namespace session
{
namespace vtk
{

Read::Result Read::operateInternal()
{
  std::string filename = this->parameters()->findFile("filename")->value();

  // Load file and parse it:
  smtk::model::SessionIOJSON::json j = smtk::model::SessionIOJSON::loadJSON(filename);
  if (j.is_null())
  {
    smtkErrorMacro(log(), "Cannot parse file \"" << filename << "\".");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  std::vector<smtk::common::UUID> preservedUUIDs;
  {
    std::vector<std::string> uuidStrs = j.at("preservedUUIDs");
    for (auto& str : uuidStrs)
    {
      preservedUUIDs.emplace_back(str);
    }
  }

  std::string resourceIdStr = j.at("id");
  smtk::common::UUID resourceId(resourceIdStr);

  // Create a new resource for the import
  auto resource = smtk::session::vtk::Resource::create();
  resource->setId(resourceId);
  auto session = smtk::session::vtk::Session::create();
  resource->setLocation(filename);
  resource->setSession(session);

  // Transcribe model data onto the resource
  auto modelResource = std::static_pointer_cast<smtk::model::Resource>(resource);
  smtk::model::from_json(j, modelResource);

  // Access all of the model files contained by the resource file.
  std::vector<std::string> modelFiles = j.at("modelFiles");

  // The file names in the smtk file are relative to the smtk file itself. We
  // need to append the smtk file's prefix to these values.
  std::string fileDirectory = smtk::common::Paths::directory(filename) + "/";

  // Import each model file listed in the resource file. The import operator
  // allows us to import models into an existing resource, so we do just that.
  Import::Ptr importOp = Import::create();
  importOp->parameters()->associations()->appendValue(resource);
  importOp->parameters()->findString("session only")->setDiscreteIndex(0);
  {
    std::vector<std::string> uuidStrs = j.at("preservedUUIDs");
    for (auto& str : uuidStrs)
    {
      importOp->m_preservedUUIDs.emplace_back(str);
    }
  }

  for (auto& modelFile : modelFiles)
  {
    importOp->parameters()->findFile("filename")->setValue(fileDirectory + modelFile);
    Result importOpResult = importOp->operate();

    if (importOpResult->findInt("outcome")->value() != static_cast<int>(Outcome::SUCCEEDED))
    {
      smtkErrorMacro(log(), "Cannot import file \"" << modelFile << "\".");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
  }

  resource->setLocation(filename);

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resourcesCreated");
    created->appendValue(resource);
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

  // Special case, if loaded from project, resource item is not set
  // Reason is not known, but this serves as a workaround
  if ((resourceItem->numberOfValues() == 1) && resourceItem->value(0) == nullptr)
  {
    return;
  }

  for (auto rit = resourceItem->begin(); rit != resourceItem->end(); ++rit)
  {
    auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(*rit);

    // Set the resource as unmodified from its persistent (i.e. on-disk) state
    resource->setClean(true);
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

} // namespace vtk
} // namespace session
} // namespace smtk
