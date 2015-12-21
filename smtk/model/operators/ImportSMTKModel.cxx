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

#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/mesh/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/io/ImportJSON.h"
#include "cJSON.h"

#include <fstream>
#include <iostream>

using namespace smtk::model;
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
  std::ifstream file(filename);
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
    cJSON* mtyp = cJSON_GetObjectItem(root, "type");
    if (mtyp && mtyp->type == cJSON_String && mtyp->valuestring &&
       !strcmp(mtyp->valuestring,"SMTK_Session"))
      {
      status = smtk::io::ImportJSON::ofLocalSession(root->child, this->manager());
      }
    }

  OperatorResult result = this->createResult( status ? OPERATION_SUCCEEDED :
                                              OPERATION_FAILED);
  // figure out new models and meshes
  if(status)
    {
    smtk::model::EntityRefArray modelModArr;
    smtk::model::EntityRefArray modelCreArr;
    smtk::model::EntityRefArray meshArr;
    smtk::mesh::ManagerPtr meshMgr = this->manager()->meshes();
    for (cJSON* sessentry = root->child; sessentry; sessentry = sessentry->next)
      {
      for (cJSON* modelentry = sessentry->child; modelentry; modelentry = modelentry->next)
        {
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
      }
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
