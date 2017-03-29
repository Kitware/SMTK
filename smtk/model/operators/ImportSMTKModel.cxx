//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/ImportSMTKModel.h"

#include "smtk/model/Session.h"

#include "smtk/mesh/Manager.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/common/CompilerInformation.h"

#include "smtk/io/ImportJSON.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include "cJSON.h"

#include <fstream>
#include <iostream>

using namespace smtk::model;
using namespace boost::filesystem;
using smtk::attribute::FileItem;
using smtk::attribute::IntItem;

namespace smtk {
  namespace model {

smtk::model::OperatorResult ImportSMTKModel::operateInternal()
{
  smtk::attribute::FileItemPtr filenameItem = this->findFile("filename");
  smtk::attribute::VoidItemPtr loadmeshItem = this->findAs<smtk::attribute::VoidItem>("loadmesh");

  std::string filename = filenameItem->value();
  if (filename.empty())
    {
    smtkErrorMacro(this->log(), "A filename must be provided.\n");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

//  smtkDebugMacro("Reading a JSON file.");
  std::ifstream file(filename.c_str());
  if (!file.good())
    {
    smtkErrorMacro(this->log(), "Could not open file \"" << filename << "\".\n");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  std::string data(
    (std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));

  if (data.empty())
    {
    smtkErrorMacro(this->log(), "No JSON objects in file\"" << filename << "\".\n");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  // cache number of models and meshes
  smtk::model::Models models =
    this->manager()->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY);

  int status = 0;
  cJSON* root = cJSON_Parse(data.c_str());
  if (root && root->type == cJSON_Object && root->child)
    {
    status = smtk::io::ImportJSON::ofLocalSession(root->child, this->manager(), true,
      path(filename).parent_path().string());
    }

  OperatorResult result = this->createResult( status ? OPERATION_SUCCEEDED :
                                              OPERATION_FAILED);
  cJSON* modelsObj = cJSON_GetObjectItem(root->child, "models");
  // figure out new models and meshes
  if(status && modelsObj)
    {
    smtk::model::EntityRefArray modelModArr;
    smtk::model::EntityRefArray modelCreArr;
    smtk::model::EntityRefArray meshArr;
    smtk::mesh::ManagerPtr meshMgr = this->manager()->meshes();

    // cache number of models and meshes
    smtk::model::Models newmodels =
      this->manager()->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY);
    smtk::model::Models::const_iterator newit;
    for(newit = newmodels.begin(); newit != newmodels.end(); ++newit)
      {
      // ignore submodels
      if(newit->parent().isModel())
        continue;
      bool existing = std::find(models.begin(), models.end(), *newit) != models.end();
      if(existing)
        modelModArr.push_back(*newit);
      else
        modelCreArr.push_back(*newit);

      if(meshMgr->associatedCollections(*newit).size() > 0)
        meshArr.push_back(*newit);
      }

/*
    // import all native models model entites, should only have meta info
    cJSON* modelentry;
    for (modelentry = modelsObj->child; modelentry; modelentry = modelentry->next)
      {
      if (!modelentry->string || !modelentry->string[0])
        continue;
      smtk::common::UUID modId(modelentry->string);
      smtk::model::Model curModel(this->manager(), modId);
      bool existing = std::find(models.begin(), models.end(), curModel) != models.end();
      if(existing)
        modelModArr.push_back(curModel);
      else
        modelCreArr.push_back(curModel);

      if(meshMgr->associatedCollections(curModel).size() > 0)
        meshArr.push_back(curModel);
      }
*/
    this->addEntitiesToResult(result, modelModArr, MODIFIED);
    this->addEntitiesToResult(result, modelCreArr, CREATED);
    result->findModelEntity("mesh_created")->setValues(meshArr.begin(), meshArr.end());
    }

  return result;
}

  } //namespace model
} // namespace smtk

#include "smtk/model/ImportSMTKModel_xml.h"

smtkImplementsModelOperator(
  SMTKCORE_EXPORT,
  smtk::model::ImportSMTKModel,
  import_smtk_model,
  "import smtk model",
  ImportSMTKModel_xml,
  smtk::model::Session);
