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
int SessionIOJSON::importJSON(ManagerPtr modelMgr,
                              const SessionPtr& session,
                              cJSON* sessionRec,
                              bool loadNativeModels)
{
  smtk::common::UUIDs models =
    modelMgr->entitiesMatchingFlags(smtk::model::MODEL_ENTITY);

  cJSON* modelsObj = cJSON_GetObjectItem(sessionRec, "models");
  if (!modelsObj)
    {
    smtkInfoMacro(modelMgr->log(), "Expecting a \"models\" entry!");
    return 0;
    }

  std::map<smtk::common::UUID, std::string> existingURLs;
  cJSON* modelentry;
  // import all native models model entites, should only have meta info
  for (modelentry = modelsObj->child; modelentry; modelentry = modelentry->next)
    {
    if (!modelentry->string || !modelentry->string[0])
      continue;

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
      // by looking at "url" property
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

      if (loadNativeModels)
        {
        std::string nativemodelfile;
        std::string nativefilekey = modelMgr->hasStringProperty(modelid, "url") ? "url" : "";
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
      }
    else if(modelMgr->hasStringProperty(modelid, "url"))
      {
      smtk::model::StringList const& nprop(modelMgr->stringProperty(modelid, "url"));
      if (!nprop.empty())
        {
        existingURLs[modelid] = nprop[0];
        }
      }
    }
  int status = this->loadModelsRecord(modelMgr, sessionRec);
  status &= this->loadMeshesRecord(modelMgr, sessionRec);
  // recover "url" property for models already loaded
  std::map<smtk::common::UUID, std::string>::const_iterator mit;
  for(mit = existingURLs.begin(); mit != existingURLs.end(); ++mit)
    {
    modelMgr->setStringProperty(mit->first, "url", mit->second);
    }
  return status;
}

/**\brief Parse models info from \a sessionRec for the given \a modelMgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
int SessionIOJSON::loadModelsRecord(ManagerPtr modelMgr,
                                    cJSON* sessionRec)
{
  cJSON* modelsObj = cJSON_GetObjectItem(sessionRec, "models");
  if (!modelsObj)
    {
    // It's okay if we don't have "models" entry. Could be a record that
    // did not come from SessionIOJSON;
    smtkInfoMacro(modelMgr->log(), "Expecting a \"models\" entry!");
    return 1;
    }

  int status = 1;
  cJSON* modelentry;
  // import all native models model entites, should only have meta info
  for (modelentry = modelsObj->child; modelentry; modelentry = modelentry->next)
    {
    if (!modelentry->string || !modelentry->string[0])
      continue;

    smtk::common::UUID modelid = smtk::common::UUID(modelentry->string);
    if (modelid.isNull())
      {
      smtkInfoMacro(modelMgr->log(), "Invalid model uuid, skipping!");
      continue;
      }
    // model meta info 
    status &= smtk::io::ImportJSON::ofManager(modelentry, modelMgr);
    }

  return status;
}

/**\brief Parse meshes info from \a sessionRec for the given \a modelMgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
int SessionIOJSON::loadMeshesRecord(ManagerPtr modelMgr,
                                    cJSON* sessionRec)
{
  cJSON* meshesObj = cJSON_GetObjectItem(sessionRec, "mesh_collections");
  if (!meshesObj)
    {
    // It's okay if we don't have "mesh_collections" entry. Could be a record that
    // did not come from SessionIOJSON;
    smtkInfoMacro(modelMgr->log(), "Expecting a \"mesh_collections\" entry!");
    return 1;
    }
  return smtk::io::ImportJSON::ofMeshesOfModel(sessionRec, modelMgr, true);
}

/**\brief Encode information into \a sessionRec for the given \a modelMgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
int SessionIOJSON::exportJSON(ManagerPtr modelMgr,
                              const SessionPtr& session,
                              cJSON* sessionRec,
                              bool writeNativeModels)
{
  (void)modelMgr;
  (void)session;
  (void)sessionRec;
  (void)writeNativeModels;
  return 1;
}

/**\brief Encode information into \a sessionRec for the given \a modelId of the \a modelMgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
int SessionIOJSON::exportJSON(
  ManagerPtr modelMgr,
  const SessionPtr& session,
  const smtk::common::UUIDs& modelIds,
  cJSON* sessionRec,
  bool writeNativeModels)
{
  if (writeNativeModels)
    {
    // We will write each model seperately:
    smtk::common::UUIDs::const_iterator modit;
    for (modit = modelIds.begin(); modit != modelIds.end(); ++modit)
      {
      smtk::model::Model model(modelMgr, *modit);
      std::string outNativeFile = this->getOutputFileNameForNativeModel(modelMgr, session, model);
      if (this->writeNativeModel(modelMgr, session, model, outNativeFile))
        {
        if (!this->referencePath().empty())
          {
          model.setStringProperty(
            "url",
            relative(outNativeFile, this->referencePath()).string());
          }
        else
          {
          model.setStringProperty("url", outNativeFile);
          }
        }
      }
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

  // the output filename could have been changed by write operator.
  outNativeFile = writeOp->specification()->findFile("filename")->value();
  return 1;
}

/**\brief Get or construct a filename for saving the native model.
  *
  * Return a full file path.
  */
std::string SessionIOJSON::getOutputFileNameForNativeModel(
  smtk::model::ManagerPtr modelMgr,
  const smtk::model::SessionPtr& sess,
  const smtk::model::Model& model) const
{
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
  outfilename << smtkfilename << "_out" << origfileext;
  return (path(smtkfilepath) / path(outfilename.str())).string();

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

    path actualFilename(inNativeFile);
    if (!this->referencePath().empty() && !actualFilename.is_absolute())
      {
      path tryme = this->referencePath() / actualFilename;
      if (exists(tryme))
        {
        actualFilename = tryme;
        }
      }
    readOp->specification()->findFile("filename")->setValue(actualFilename.string());
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
