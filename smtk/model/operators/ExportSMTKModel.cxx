//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/ExportSMTKModel.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/io/ExportJSON.h"
#include "smtk/io/ExportJSON.txx"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/model/Session.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "boost/filesystem.hpp"
#include "cJSON.h"
#include <fstream>

using namespace smtk::model;
using namespace boost::filesystem;

namespace smtk {
  namespace model {

smtk::model::OperatorResult ExportSMTKModel::operateInternal()
{
  smtk::attribute::FileItemPtr filenameItem = this->findFile("filename");
  smtk::attribute::IntItemPtr flagsItem = this->findInt("flags");

  smtk::model::Models models = this->m_specification->associatedModelEntities<smtk::model::Models>();
  if (models.empty())
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

  cJSON* top = cJSON_CreateObject();
  std::string smtkfilepath = path(filename).parent_path().string();
  std::string smtkfilename = path(filename).stem().string();

  // Add the output smtk model name to the model "smtk_url", so that the individual session can
  // use that name to construct a filename for saving native models of the session.
  smtk::model::Models::const_iterator modit;
  for(modit = models.begin(); modit != models.end(); ++modit)
    {
    this->manager()->setStringProperty(modit->entity(), "smtk_url", filename);

    // we also want to write out the meshes to new "write_locations"
    std::vector<smtk::mesh::CollectionPtr> collections =
      this->manager()->meshes()->associatedCollections(*modit);
    std::vector<smtk::mesh::CollectionPtr>::const_iterator cit;
    for(cit = collections.begin(); cit != collections.end(); ++cit)
      {
      std::ostringstream outmeshname;
      outmeshname << smtkfilename << "_" << (*cit)->name() << ".h5m";
      std::string write_path = (path(smtkfilepath) / path(outmeshname.str())).string();
      (*cit)->writeLocation(write_path);
      }
    }

  smtk::io::ExportJSON::forManagerSessionPartial(
    this->session()->sessionId(),
    this->m_specification->associatedModelEntityIds(),
    top, this->manager(), true, smtkfilepath);

  char* json = cJSON_Print(top);
  jsonFile << json;
  free(json);
  cJSON_Delete(top);
  jsonFile.close();

  return this->createResult(smtk::model::OPERATION_SUCCEEDED);
}

  } //namespace model
} // namespace smtk

#include "smtk/model/ExportSMTKModel_xml.h"

smtkImplementsModelOperator(
  SMTKCORE_EXPORT,
  smtk::model::ExportSMTKModel,
  export_smtk_model,
  "export smtk model",
  ExportSMTKModel_xml,
  smtk::model::Session);
