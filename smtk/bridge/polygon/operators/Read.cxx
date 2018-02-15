//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "Read.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/bridge/polygon/Resource.h"
#include "smtk/bridge/polygon/SessionIOJSON.h"

#include "smtk/bridge/polygon/Read_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
{
namespace polygon
{

Read::Result Read::operateInternal()
{
  std::string filename = this->parameters()->findFile("filename")->value();

  // Load file and parse it:
  smtk::bridge::polygon::SessionIOJSON::json j =
    smtk::bridge::polygon::SessionIOJSON::loadJSON(filename);
  if (j.is_null())
  {
    smtkErrorMacro(log(), "Cannot parse file \"" << filename << "\".");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Deserialize parsed JSON into a model resource:
  auto rsrc = smtk::bridge::polygon::Resource::create();
  smtk::bridge::polygon::SessionIOJSON::loadModelRecords(j, rsrc);

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->setValue(rsrc);
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

} // namespace polygon
} // namespace bridge
} // namespace smtk
