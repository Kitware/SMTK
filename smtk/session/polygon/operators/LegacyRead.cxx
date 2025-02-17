//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "LegacyRead.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/SessionIOJSON.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/session/polygon/operators/LegacyRead_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace session
{
namespace polygon
{

LegacyRead::Result LegacyRead::operateInternal()
{
  std::string filename = this->parameters()->findFile("filename")->value();

  // Load file and parse it:
  smtk::session::polygon::SessionIOJSON::json j =
    smtk::session::polygon::SessionIOJSON::loadJSON(filename);
  if (j.is_null())
  {
    smtkErrorMacro(log(), "Cannot parse file \"" << filename << "\".");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Deserialize parsed JSON into a model resource:
  auto rsrc = smtk::session::polygon::Resource::create();
  rsrc->setId(j.begin().key());
  smtk::session::polygon::SessionIOJSON::loadModelRecords(*j.begin(), rsrc);
  rsrc->setLocation(filename);

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resourcesCreated");
    created->appendValue(rsrc);
  }

  operation::MarkGeometry markGeometry(rsrc);
  smtk::resource::Component::Visitor visitor = [&markGeometry](const resource::ComponentPtr& comp) {
    markGeometry.markModified(comp);
  };
  rsrc->visit(visitor);

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
  return result->findResource("resourcesCreated")->value();
}

} // namespace polygon
} // namespace session
} // namespace smtk
