//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/ExportModelJSON.h"

#include "smtk/model/Session.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Model.h"
#include "smtk/model/json/jsonEntity.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/model/operators/ExportModelJSON_xml.h"

#include <fstream>

using namespace smtk::model;

namespace smtk
{
namespace model
{

ExportModelJSON::Result ExportModelJSON::operateInternal()
{
  smtk::attribute::FileItemPtr filenameItem = this->parameters()->findFile("filename");
  smtk::attribute::IntItemPtr flagsItem = this->parameters()->findInt("flags");

  auto associations = this->parameters()->associations();
  auto entities = associations->as<std::vector<smtk::model::Entity::Ptr>>(
    [](smtk::resource::PersistentObjectPtr obj) {
      return std::dynamic_pointer_cast<smtk::model::Entity>(obj);
    });
  if (entities.empty())
  {
    smtkErrorMacro(this->log(), "No valid models selected for export.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  std::string filename = filenameItem->value();
  if (filename.empty())
  {
    smtkErrorMacro(this->log(), "A filename must be provided.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  std::ofstream jsonFile(filename.c_str(), std::ios::trunc);
  if (!jsonFile.good())
  {
    smtkErrorMacro(this->log(), "Could not open file \"" << filename << "\".");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // TODO: incorporate JSON flags into nlohmann::json bindings
  // JSONFlags flags = static_cast<JSONFlags>(flagsItem->value(0));

  // std::string jsonStr =
  //   smtk::io::SaveJSON::forEntities(entities, smtk::model::ITERATE_MODELS, flags);

  nlohmann::json json = entities;

  jsonFile << json;
  jsonFile.close();

  return this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
}

const char* ExportModelJSON::xmlDescription() const
{
  return ExportModelJSON_xml;
}

} //namespace model
} // namespace smtk
