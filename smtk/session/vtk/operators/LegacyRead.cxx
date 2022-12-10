//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/vtk/operators/LegacyRead.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/session/vtk/Resource.h"
#include "smtk/session/vtk/json/jsonResource.h"
#include "smtk/session/vtk/operators/Import.h"
#include "smtk/session/vtk/operators/LegacyRead_xml.h"

#include "smtk/common/Paths.h"

#include "smtk/model/SessionIOJSON.h"

using namespace smtk::model;

namespace smtk
{
namespace session
{
namespace vtk
{

LegacyRead::Result LegacyRead::operateInternal()
{
  std::string filename = this->parameters()->findFile("filename")->value();

  // Load file and parse it.
  smtk::model::SessionIOJSON::json j = smtk::model::SessionIOJSON::loadJSON(filename);
  if (j.is_null())
  {
    smtkErrorMacro(log(), "Cannot parse file \"" << filename << "\".");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  auto& jsonData = *j.begin();

  // Create a new resource for the import
  auto resource = smtk::session::vtk::Resource::create();
  auto session = smtk::session::vtk::Session::create();
  resource->setLocation(filename);
  resource->setSession(session);

  // Access all of the model files contained by the resource file.
  std::vector<std::string> modelFiles = jsonData.at("modelFiles");

  // The file names in the smtk file are relative to the smtk file itself. We
  // need to append the smtk file's prefix to these values.
  std::string fileDirectory = smtk::common::Paths::directory(filename) + "/";

  // Import each model file listed in the resource file. The import operator
  // allows us to import models into an existing resource, so we do just that.
  Import::Ptr importOp = Import::create();
  importOp->parameters()->associations()->appendValue(resource);
  importOp->parameters()->findString("session only")->setDiscreteIndex(0);
  {
    std::vector<std::string> uuidStrs = jsonData.at("preservedUUIDs");
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
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->setValue(resource);
  }

  return result;
}

const char* LegacyRead::xmlDescription() const
{
  return LegacyRead_xml;
}

smtk::resource::ResourcePtr legacyRead(const std::string& filename)
{
  LegacyRead::Ptr legacyRead = LegacyRead::create();
  legacyRead->parameters()->findFile("filename")->setValue(filename);
  LegacyRead::Result result = legacyRead->operate();
  if (result->findInt("outcome")->value() != static_cast<int>(LegacyRead::Outcome::SUCCEEDED))
  {
    return smtk::resource::ResourcePtr();
  }
  return result->findResource("resource")->value();
}

} // namespace vtk
} // namespace session
} // namespace smtk
