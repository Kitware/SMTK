//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/SessionIOJSON.h"

#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/common/UUID.h"
#include "smtk/io/ExportJSON.h"
#include "smtk/io/ExportJSON.txx"
#include "smtk/io/ImportJSON.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"

#include "boost/filesystem.hpp"
#include "cJSON.h"

using namespace boost::filesystem;

namespace smtk {
  namespace model {

/**\brief Decode information from \a sessionRec for the given \a modelMgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
// ++ 2 ++
int SessionIOJSON::importJSON(ManagerPtr modelMgr, cJSON* sessionRec)
{
  (void)modelMgr;
  (void)sessionRec;
  return 1;
}

/**\brief Decode information from \a sessionRec for the given \a modelMgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
int SessionIOJSON::importJSON(ManagerPtr modelMgr,
                              const SessionPtr& session,
                              cJSON* sessionRec)
{
  int status = 0;
  smtk::common::UUIDs models =
    modelMgr->entitiesMatchingFlags(smtk::model::MODEL_ENTITY);
  cJSON* modelentry;
  // import all model entites, should only have meta info
  for (modelentry = sessionRec->child; modelentry; modelentry = modelentry->next)
    {
    smtk::common::UUID modelid = smtk::common::UUID(modelentry->string);
    if (modelid.isNull())
      {
      smtkInfoMacro(modelMgr->log(), "Invalid model uuid, skipping!");
      continue;
      }
    // import native model if the model does not exist;
    // NOTE: what should we do if it already exists? erase then re-load
    // the original model from file (stored in string property "url")?
    // Else, just import meta info
    if(models.find(modelid) == models.end())
      {
      // find the model entry, and get the native model file name if it exists,
      // by looking at "output_native_url" property
      for (cJSON* curChild = modelentry->child; curChild; curChild = curChild->next)
        {
        if (!curChild->string || !curChild->string[0])
          {
          continue;
          }
        // find the model id in dictionary
        if (smtk::common::UUID(curChild->string) != modelid)
          {
          continue;
          }
        // failed to load properties is still OK
        smtk::io::ImportJSON::ofManagerStringProperties(modelid, curChild, modelMgr);
        break;
        }

      std::string nativemodelfile;
      std::string nativefilekey = modelMgr->hasStringProperty(modelid, "output_native_url") ?
                                  "output_native_url" :
                                  (modelMgr->hasStringProperty(modelid, "url") ? "url" : "");
      if (!nativefilekey.empty())
        {
        smtk::model::StringList const& nprop(modelMgr->stringProperty(modelid, nativefilekey));
        if (!nprop.empty())
          {
          nativemodelfile = nprop[0];
          }
        }

      if(!nativemodelfile.empty())
        {
        // failed to load native model is still ok
        this->loadNativeModel(modelMgr, session, nativemodelfile);
        }
      }

    // model meta info 
    status &= smtk::io::ImportJSON::ofManager(modelentry, modelMgr);
    // mesh collections related to this model
    status &= smtk::io::ImportJSON::ofMeshesOfModel(modelentry, modelMgr, true);
    }

  return status;
}

/**\brief Encode information into \a sessionRec for the given \a modelMgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
int SessionIOJSON::exportJSON(ManagerPtr modelMgr, cJSON* sessionRec)
{
  (void)modelMgr;
  (void)sessionRec;
  return 1;
}

/**\brief Encode information into \a sessionRec for the given \a modelId of the \a modelMgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
int SessionIOJSON::exportJSON(ManagerPtr modelMgr,
                              const SessionPtr& session,
                              const smtk::common::UUIDs& modelIds,
                              cJSON* sessionRec)
{
  // we will write each model seperately
  smtk::common::UUIDs::const_iterator modit;
  for(modit = modelIds.begin(); modit != modelIds.end(); ++modit)
    {
    smtk::model::Model model(modelMgr, *modit);
    std::string outNativeFile;
    if(this->writeNativeModel(
      modelMgr, session, model, outNativeFile))
      {
      modelMgr->setStringProperty(*modit, "output_native_url", outNativeFile);
      }

    cJSON* jmodel = cJSON_CreateObject();

    cJSON_AddStringToObject(jmodel, "type", "model");
    cJSON_AddStringToObject(jmodel, "name", model.name().c_str());
    // Write out all entities of the model, only the meta data
    smtk::model::Models currentmodels;
    currentmodels.push_back(model);
    smtk::io::ExportJSON::forEntities(jmodel, currentmodels,
                                      smtk::model::ITERATE_MODELS,
                                      smtk::io::JSON_CLIENT_DATA);

    // Write out related mesh collections.
    // When writing a single collection, all its MeshSets will also be written out.
    smtk::io::ExportJSON::forModelMeshes(*modit, jmodel, modelMgr);

    cJSON_AddItemToObject(sessionRec, modit->toString().c_str(), jmodel);
    }
  return 1;
}

/**\brief Create and write to a file (\a outNativeFile) for the given \a model.
  *
  * Return 1 on success and 0 on failure.
  */
int SessionIOJSON::writeNativeModel(smtk::model::ManagerPtr modelMgr,
                              const smtk::model::SessionPtr& sess,
                              const smtk::model::Model& model,
                              std::string& outNativeFile)
{
  // if this is not a valid session, return;
  if(!sess)
    {
    smtkInfoMacro(modelMgr->log(), "Expecting a valid session!");
    return 0;
    }

  std::string smtkfilepath, smtkfilename, origfilename, origfileext;
  if (modelMgr->hasStringProperty(model.entity(), "smtk_url"))
    {
    smtk::model::StringList const& nprop(modelMgr->stringProperty(model.entity(), "smtk_url"));
    if (!nprop.empty())
      {
      smtkfilepath = path(nprop[0]).parent_path().string();
      smtkfilename = path(nprop[0]).stem().string();
      }
    }

  if (modelMgr->hasStringProperty(model.entity(), "url"))
    {
    smtk::model::StringList const& nprop(modelMgr->stringProperty(model.entity(), "url"));
    if (!nprop.empty())
      {
      origfilename = path(nprop[0]).stem().string();
      origfileext = path(nprop[0]).extension().string();
      }
    }
  if (smtkfilename.empty() && !origfilename.empty())
    smtkfilename = origfilename;

  if (smtkfilename.empty())
    smtkfilename = sess->name();

  std::ostringstream outfilename;
  outfilename << smtkfilename << "_native" << origfileext;
  outNativeFile = (path(smtkfilepath) / path(outfilename.str())).string();

  smtk::model::OperatorPtr writeOp = sess->op("write");
  if (!writeOp)
    {
    smtkInfoMacro(modelMgr->log(), "Failed to create a write operator to write the model for native kernel!");
    return 0;
    }
  writeOp->specification()->findFile("filename")->setValue(outNativeFile);
  writeOp->specification()->associateEntity(model);

  smtk::model::OperatorResult opresult = writeOp->operate();
  if (opresult->findInt("outcome")->value() !=
      smtk::model::OPERATION_SUCCEEDED)
    {
    smtkInfoMacro(modelMgr->log(), "Failed to write the model for native kernel!");
    return 0;
    }

  return 1;
}

/**\brief Load the native model given the filename (\a inNativeFile).
  *
  * Return 1 on success and 0 on failure.
  */
int SessionIOJSON::loadNativeModel(smtk::model::ManagerPtr modelMgr,
                              const smtk::model::SessionPtr& sess,
                              const std::string& inNativeFile)
{
  // if this is not a valid session, return;
  if(!sess)
    {
    smtkInfoMacro(modelMgr->log(), "Expecting a valid session!");
    return 0;
    }

  if (!inNativeFile.empty())
    {
    smtk::model::OperatorPtr readOp = sess->op("read");
    if (!readOp)
      {
      smtkInfoMacro(modelMgr->log(), "Failed to create a read operator to read the model for native kernel!");
      return 0;
      }
    readOp->specification()->findFile("filename")->setValue(inNativeFile);
    smtk::model::OperatorResult opresult = readOp->operate();
    if (opresult->findInt("outcome")->value() !=
        smtk::model::OPERATION_SUCCEEDED)
      {
      smtkInfoMacro(modelMgr->log(), "Failed to read the model for native kernel!");
      return 0;
      }
    return 1;
    }

  return 0;
}

  } // namespace model
} // namespace smtk
