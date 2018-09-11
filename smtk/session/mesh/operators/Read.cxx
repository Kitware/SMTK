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

#include "smtk/session/mesh/Resource.h"

#include "smtk/session/mesh/Read_xml.h"
#include "smtk/session/mesh/operators/Import.h"

#include "smtk/common/CompilerInformation.h"

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

  std::ifstream file(filename);
  if (!file.good())
  {
    smtkErrorMacro(log(), "Cannot read file \"" << filename << "\".");
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
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Access the resource's id
  std::string resourceIdStr = j.at("id");
  smtk::common::UUID resourceId(resourceIdStr);

  // Create a new resource for the import
  auto resource = smtk::session::mesh::Resource::create();
  resource->setId(resourceId);
  auto session = smtk::session::mesh::Session::create();
  resource->setLocation(filename);
  resource->setSession(session);

  std::string meshFilename = j.at("Mesh URL");

  // Create an import operator
  smtk::session::mesh::Import::Ptr importOp = smtk::session::mesh::Import::create();
  importOp->parameters()->findResource("resource")->setIsEnabled(true);
  importOp->parameters()->findResource("resource")->setValue(resource);
  importOp->parameters()->findString("session only")->setDiscreteIndex(0);
  importOp->parameters()->findFile("filename")->setValue(meshFilename);

  // Execute the operation
  smtk::operation::Operation::Result importOpResult = importOp->operate();

  // Retrieve the resulting resource
  smtk::attribute::ResourceItemPtr resourceItem =
    std::dynamic_pointer_cast<smtk::attribute::ResourceItem>(
      importOpResult->findResource("resource"));

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->setValue(resourceItem->value());
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
}
}
}
