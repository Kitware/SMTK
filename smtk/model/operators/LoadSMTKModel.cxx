//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/LoadSMTKModel.h"

#include "smtk/model/Session.h"

#include "smtk/mesh/core/Manager.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/common/CompilerInformation.h"

#include "smtk/io/LoadJSON.h"

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

static void updateSMTKURLs(cJSON* parent, cJSON* node, const std::string& filename)
{
  for (; node; node = node->next)
  {
    if (parent && parent->type == cJSON_Object && node->type == cJSON_Array && node->string &&
      node->string[0] && std::string(node->string) == "smtk_url")
    {
      cJSON* replacement = cJSON_CreateString(filename.c_str());
      cJSON_ReplaceItemInArray(node, 0, replacement);
    }
    else if ((node->type == cJSON_Object || node->type == cJSON_Array) && node->child)
    {
      updateSMTKURLs(node, node->child, filename);
    }
  }
}

static void updateURLs(cJSON* parent, cJSON* node, const ::boost::filesystem::path& embedDir)
{
  for (; node; node = node->next)
  {
    if (parent && parent->type == cJSON_Object && node->type == cJSON_Array && node->string &&
      node->string[0] &&
      (std::string(node->string) == "url" || std::string(node->string) == "modelFiles"))
    {
      int nn = 0;
      for (cJSON *urlNode = node->child; urlNode; urlNode = urlNode->next, ++nn)
      {
        if (urlNode->valuestring && urlNode->valuestring[0])
        {
          ::boost::filesystem::path url(urlNode->valuestring);
          if (url.is_relative())
          {
            cJSON* replacement = cJSON_CreateString((embedDir / url).string().c_str());
            cJSON_ReplaceItemInArray(node, nn, replacement);
            urlNode = replacement;
          }
        }
      }
    }
    else if ((node->type == cJSON_Object || node->type == cJSON_Array) && node->child)
    {
      updateURLs(node, node->child, embedDir);
    }
  }
}

static void replaceUUID(cJSON* node, const char* original, const char* replacement)
{
  if (node->string && node->string[0] && !strncmp(original, node->string, 36))
  {
    strncpy(node->string, replacement, 36);
  }
  if (node->type == cJSON_String && !strncmp(original, node->valuestring, 36))
  {
    strncpy(node->valuestring, replacement, 36);
  }
  else if (node->child)
  {
    for (cJSON* child = node->child; child; child = child->next)
    {
      replaceUUID(child, original, replacement);
    }
  }
}

static bool updateSessionID(cJSON* root, const smtk::model::SessionRef& curSess)
{
  bool didReplace = false;
  if (!root || root->type != cJSON_Object)
    return didReplace;

  std::string replaceStr(curSess.entity().toString()); // Only convert UUID to string once.
  for (cJSON* sessNode = root->child; sessNode; sessNode = sessNode->next)
  {
    smtk::common::UUID nodeUUID;
    if (sessNode->string && sessNode->string[0] && strlen(sessNode->string) == 36 &&
      (nodeUUID = smtk::common::UUID(sessNode->string)) && sessNode->type == cJSON_Object)
    {
      cJSON* ntype = cJSON_GetObjectItem(sessNode, "type");
      cJSON* sname = cJSON_GetObjectItem(sessNode, "name");
      if (ntype && ntype->type == cJSON_String && ntype->valuestring && ntype->valuestring[0] &&
        std::string(ntype->valuestring) == "session" && sname && sname->type == cJSON_String &&
        sname->valuestring && sname->valuestring[0] &&
        curSess.session()->name() == sname->valuestring)
      { // The node's key is a valid UUID and its contents indicate it is a session of the same
        // type as curSess.
        std::string originalStr(sessNode->string);
        if (originalStr != replaceStr)
        {
          replaceUUID(sessNode, originalStr.c_str(), replaceStr.c_str());
          didReplace = true;
        }
      }
    }
  }
  return didReplace;
}

namespace smtk
{
namespace model
{

smtk::model::OperatorResult LoadSMTKModel::operateInternal()
{
  smtk::attribute::FileItemPtr filenameItem = this->findFile("filename");
  smtk::attribute::VoidItemPtr loadmeshItem = this->findAs<smtk::attribute::VoidItem>("loadmesh");

  std::string filename = filenameItem->value();
  if (filename.empty())
  {
    smtkErrorMacro(this->log(), "A filename must be provided.\n");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  //  smtkDebugMacro("Reading a JSON file.");
  std::ifstream file(filename.c_str());
  if (!file.good())
  {
    smtkErrorMacro(this->log(), "Could not open file \"" << filename << "\".\n");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  std::string data((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));

  if (data.empty())
  {
    smtkErrorMacro(this->log(), "No JSON objects in file\"" << filename << "\".\n");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  // cache number of models and meshes
  smtk::model::Models models =
    this->manager()->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY);

  int status = 0;
  cJSON* root = cJSON_Parse(data.c_str());
  // Rewrite the JSON to use absolute paths:
  updateSMTKURLs(NULL, root, filename);
  ::boost::filesystem::path embedDir = ::boost::filesystem::path(filename).parent_path();
  if (!embedDir.empty())
  {
    updateURLs(NULL, root, embedDir);
  }
  smtk::model::SessionPtr curSessObj = this->session();
  if (root && root->type == cJSON_Object && root->child && curSessObj)
  {
    smtk::model::SessionRef curSess(this->manager(), curSessObj->sessionId());
    if (curSess.isValid())
    {
      // Rewrite the JSON so that the stored session UUID
      // is replaced with this pre-existing session's UUID.
      updateSessionID(root, curSess);
    }
    status = smtk::io::LoadJSON::ofLocalSession(
      root->child, this->manager(), true, path(filename).parent_path().string());
  }

  OperatorResult result = this->createResult(status ? OPERATION_SUCCEEDED : OPERATION_FAILED);
  cJSON* modelsObj = cJSON_GetObjectItem(root->child, "models");
  // figure out new models and meshes
  if (status && modelsObj)
  {
    smtk::model::EntityRefArray modelCreArr;
    smtk::model::EntityRefArray meshArr;
    smtk::mesh::ManagerPtr meshMgr = this->manager()->meshes();

    // cache number of models and meshes
    smtk::model::Models newmodels =
      this->manager()->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY);
    smtk::model::Models::const_iterator newit;
    std::set<smtk::model::SessionRef> otherSessions;
    for (newit = newmodels.begin(); newit != newmodels.end(); ++newit)
    {
      // ignore submodels
      if (newit->parent().isModel())
        continue;
      bool existing = std::find(models.begin(), models.end(), *newit) != models.end();
      if (!existing)
      {
        modelCreArr.push_back(*newit);

        if (newit->session().session() != this->session())
        {
          if (newit->session().isValid())
          {
            otherSessions.insert(newit->session());
          }
          newit->as<smtk::model::Model>().setSession(
            smtk::model::SessionRef(newit->manager(), this->session()->sessionId()));
        }

        if (meshMgr->associatedCollections(*newit).size() > 0)
          meshArr.push_back(*newit);
      }
    }
    for (auto importedSess : otherSessions)
    {
      importedSess.manager()->hardErase(importedSess);
    }

    this->addEntitiesToResult(result, modelCreArr, CREATED);
    result->findModelEntity("mesh_created")->setValues(meshArr.begin(), meshArr.end());
  }

  return result;
}

} //namespace model
} // namespace smtk

#include "smtk/model/LoadSMTKModel_xml.h"

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::model::LoadSMTKModel, load_smtk_model,
  "load smtk model", LoadSMTKModel_xml, smtk::model::Session);
