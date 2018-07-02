//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/vtk/operators/Read.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/vtk/Read_xml.h"
#include "smtk/bridge/vtk/Resource.h"
#include "smtk/bridge/vtk/json/jsonResource.h"
#include "smtk/bridge/vtk/operators/Import.h"

#include "smtk/common/Paths.h"

#include "smtk/model/SessionIOJSON.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
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
      preservedUUIDs.push_back(smtk::common::UUID(str));
    }
  }

  // Initialize a monotonically increasing counter for assigning different
  // blocks preserved UUIDs.
  int curId = 0;

  // Create a new resource for the import
  auto resource = smtk::bridge::vtk::Resource::create();
  auto session = smtk::bridge::vtk::Session::create();
  resource->setLocation(filename);
  resource->setSession(session);

  // Access all of the model files contained by the resource file.
  std::vector<std::string> modelFiles = j.at("modelFiles");

  // The file names in the smtk file are relative to the smtk file itself. We
  // need to append the smtk file's prefix to these values.
  std::string fileDirectory = smtk::common::Paths::directory(filename) + "/";

  // Import each model file listed in the resource file. The import operator
  // allows us to import models into an existing resource, so we do just that.
  Import::Ptr importOp = Import::create();
  importOp->parameters()->findResource("resource")->setIsEnabled(true);
  importOp->parameters()->findResource("resource")->setValue(resource);
  importOp->parameters()->findString("session only")->setDiscreteIndex(0);
  {
    std::vector<std::string> uuidStrs = j.at("preservedUUIDs");
    for (auto& str : uuidStrs)
    {
      importOp->m_preservedUUIDs.push_back(smtk::common::UUID(str));
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

const char* Read::xmlDescription() const
{
  return Read_xml;
}

smtk::resource::ResourcePtr read(const std::string& filename)
{
  Read::Ptr read = Read::create();
  read->parameters()->findFile("filename")->setValue(filename);
  Read::Result result = read->operate();
  if (result->findInt("outcome")->value() != static_cast<int>(Read::Outcome::SUCCEEDED))
  {
    return smtk::resource::ResourcePtr();
  }
  return result->findResource("resource")->value();
}

} // namespace vtk
} // namespace bridge
} // namespace smtk
