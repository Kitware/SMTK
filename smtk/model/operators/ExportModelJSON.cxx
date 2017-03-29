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
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/io/SaveJSON.h"
#include "smtk/io/SaveJSON.txx"

#include <fstream>

using namespace smtk::model;
using smtk::attribute::FileItem;
using smtk::attribute::IntItem;
using smtk::io::JSONFlags;

namespace smtk {
  namespace model {

smtk::model::OperatorResult ExportModelJSON::operateInternal()
{
  smtk::attribute::FileItemPtr filenameItem = this->findFile("filename");
  smtk::attribute::IntItemPtr flagsItem = this->findInt("flags");

  Models entities = this->associatedEntitiesAs<Models>();
  if (entities.empty())
    {
    smtkErrorMacro(this->log(), "No valid models selected for export.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  std::string filename = filenameItem->value();
  if (filename.empty())
    {
    smtkErrorMacro(this->log(), "A filename must be provided.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  std::ofstream jsonFile(filename.c_str(), std::ios::trunc);
  if (!jsonFile.good())
    {
    smtkErrorMacro(this->log(), "Could not open file \"" << filename << "\".");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  JSONFlags flags = static_cast<JSONFlags>(flagsItem->value(0));
  std::string jsonStr = smtk::io::SaveJSON::forEntities(
    entities, smtk::model::ITERATE_MODELS, flags);

  jsonFile << jsonStr;
  jsonFile.close();

  return this->createResult(smtk::model::OPERATION_SUCCEEDED);
}

  } //namespace model
} // namespace smtk

#include "smtk/model/ExportModelJSON_xml.h"

smtkImplementsModelOperator(
  SMTKCORE_EXPORT,
  smtk::model::ExportModelJSON,
  export_model_json,
  "export model json",
  ExportModelJSON_xml,
  smtk::model::Session);
