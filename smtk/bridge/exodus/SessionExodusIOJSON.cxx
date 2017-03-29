//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/exodus/SessionExodusIOJSON.h"

#include "smtk/bridge/exodus/Session.h"

#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/io/SaveJSON.h"
#include "smtk/io/LoadJSON.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include "vtkDataObject.h"
#include "vtkMultiBlockDataSet.h"

#include "cJSON.h"

using namespace boost::filesystem;

namespace smtk {
  namespace bridge {
    namespace exodus {

/// Construct an I/O delegate.
SessionIOJSON::SessionIOJSON()
{
}

/**\brief Load a model file while preserving UUIDs contained in the JSON file in traversal-order.
  */
int SessionIOJSON::loadExodusFileWithUUIDs(
  const model::SessionRef& sref,
  const std::string& url,
  const common::UUIDArray& preservedUUIDs
)
{
  if (!sref.isValid() || url.empty())
    {
    smtkWarningMacro(
      sref.manager()->log(),
      "Invalid session (" << sref.name() << ") or URL (" << url << ")");
    return 0;
    }

  // See if we can turn a relative path into an absolute one (but only if it exists)
  path absURL(url);
  if (!this->referencePath().empty() && !absURL.is_absolute())
    {
    path tryme = this->referencePath() / absURL;
    if (exists(tryme))
      {
      absURL = canonical(tryme, this->referencePath());
      }
    }

  smtk::model::OperatorPtr readOp = sref.op("read");
  if (!readOp)
    {
    smtkInfoMacro(sref.manager()->log(), "Failed to create a read operator to read the model for native kernel!");
    return 0;
    }
  readOp->specification()->findFile("filename")->setValue(absURL.string());
  attribute::ModelEntityItem::Ptr pu = readOp->specification()->findModelEntity("preservedUUIDs");
  pu->setNumberOfValues(static_cast<int>(preservedUUIDs.size()));
  pu->setIsEnabled(preservedUUIDs.empty() ? false : true);
  int i = 0;
  for (common::UUIDArray::const_iterator uit = preservedUUIDs.begin(); uit != preservedUUIDs.end(); ++uit, ++i)
    {
    pu->setValue(i, model::EntityRef(sref.manager(), *uit));
    }
  smtk::model::OperatorResult opresult = readOp->operate();
  if (opresult->findInt("outcome")->value() !=
    smtk::model::OPERATION_SUCCEEDED)
    {
    smtkWarningMacro(sref.manager()->log(), "Failed to read the model for native kernel!");
    return 0;
    }
  return 1;
}

/**\brief Decode information from \a sessionRec for the given \a modelMgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
int SessionIOJSON::importJSON(
  model::ManagerPtr modelMgr,
  const model::SessionPtr& session,
  cJSON* sessionRec,
  bool loadNativeModels)
{
  (void)modelMgr;
  (void)session;
  (void)sessionRec;
  cJSON* preservedUUIDs = cJSON_GetObjectItem(sessionRec, "preservedUUIDs");
  cJSON* modelFiles = cJSON_GetObjectItem(sessionRec, "modelFiles");
  cJSON* models = cJSON_GetObjectItem(sessionRec, "models");

  common::UUIDArray uids;
  if (preservedUUIDs)
    {
    smtk::io::LoadJSON::getUUIDArrayFromJSON(preservedUUIDs->child, uids);
    }

  smtk::model::BitFlags whatToImport;
  if (loadNativeModels)
    {
    whatToImport = smtk::model::SESSION_EVERYTHING;
    for (cJSON* entry = modelFiles->child; entry; entry = entry->next)
      {
      smtkDebugMacro(modelMgr->log(), "Loading file \"" << entry->valuestring << "\"");
      this->loadExodusFileWithUUIDs(model::SessionRef(modelMgr, session), entry->valuestring, uids);
      }
    }
  else
    {
    whatToImport = smtk::model::SESSION_PROPERTIES;
    }
  for (cJSON* entry = models ? models->child : NULL; entry; entry = entry->next)
    {
    smtk::io::LoadJSON::ofManagerEntityData(entry, modelMgr, whatToImport);
    }

  return 1;
}

/**\brief Encode information into \a sessionRec for the given \a modelMgr.
  *
  * In this case, we want to preserve UUIDs even when the original file (with
  * UUIDs embedded) is not written to disk so that loading the session
  * starting with the JSON file results in the same UUIDs as we have currently.
  * This assumes model entities are loaded in the same order each time the
  * native file is read.
  *
  * Returns 1 on success and 0 on failure.
  */
// ++ 1 ++
int SessionIOJSON::exportJSON(
  model::ManagerPtr modelMgr,
  const model::SessionPtr& session,
  cJSON* sessionRec,
  bool writeNativeModels)
{
  model::SessionRef sref(modelMgr, session);
  model::Models sessModels = sref.models<model::Models>();
  common::UUIDs modelIds;
  model::EntityRef::EntityRefsToUUIDs(modelIds, sessModels);
  return this->exportJSON(modelMgr, session, modelIds, sessionRec, writeNativeModels);
}
// -- 1 --

/**\brief Encode information into \a sessionRec for the given \a modelMgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  * This variant should export only information for the given models.
  */
// ++ 3 ++
int SessionIOJSON::exportJSON(model::ManagerPtr modelMgr, const model::SessionPtr& session,
                         const common::UUIDs& modelIds, cJSON* sessionRec,
                         bool writeNativeModels)
{
  // We ignore writeNativeModels because we cannot write native models yet
  // (the write operator only handles label maps, not Exodus or SLAC).
  (void)writeNativeModels;

  std::vector<smtk::common::UUID> uuidArray;
  model::SessionRef sref(modelMgr, session);
  std::vector<long> toplevelOffsets;
  std::vector<long> modelNumbers;
  std::set<std::string> modelFiles;
  path refPath(this->referencePath());
  long modelNumber = 0;
  model::Models sessModels = sref.models<model::Models>();
  for (model::Models::iterator mit = sessModels.begin(); mit != sessModels.end(); ++mit, ++modelNumber)
    {
    // Only add a model
    if (modelIds.find(mit->entity()) != modelIds.end())
      {
      modelNumbers.push_back(modelNumber);
      toplevelOffsets.push_back(static_cast<long>(uuidArray.size()));
      this->addModelUUIDs(*mit, uuidArray);
      if (mit->hasStringProperty("url"))
        {
        path url(mit->stringProperty("url")[0]);
        if (!refPath.string().empty())
          {
          boost::system::error_code err;
          path tryme = relative(url, refPath, err);
          if (err == boost::system::errc::success)
            {
            url = tryme.string();
            }
          }
        // set the url property to be consistent with "modelFiles" record when written out
        mit->setStringProperty("url", url.string());
        modelFiles.insert(url.string());
        }
      }
    }

  cJSON_AddItemToObject(sessionRec, "preservedUUIDs",
    smtk::io::SaveJSON::createUUIDArray(uuidArray));
  cJSON_AddItemToObject(sessionRec, "toplevelOffsets",
    smtk::io::SaveJSON::createIntegerArray(toplevelOffsets));
  cJSON_AddItemToObject(sessionRec, "modelNumbers",
    smtk::io::SaveJSON::createIntegerArray(modelNumbers));
  std::vector<std::string> urlArray(modelFiles.begin(), modelFiles.end());
  cJSON_AddItemToObject(sessionRec, "modelFiles",
    smtk::io::SaveJSON::createStringArray(urlArray));

  return 1;
}
// -- 3 --

/**\brief Add UUIDs of children to \a uuids array.
  *
  * The children are assumed to be listed in a stable order across file loads.
  */
void SessionIOJSON::addModelUUIDs(const model::EntityRef& parent, common::UUIDArray& uuids)
{
  // Traverse entities in exactly the same way that they are
  // traversed by the read operator, to avoid causing deserialization problems.
  // Unfortunately that means this class peeks at the underlying VTK objects
  // which should be hidden by the "handle" abstraction. We accept the leaky
  // abstraction in order to ensure the traversal results are proper.
  smtk::model::SessionRef sref = parent.isModel() ?
    parent.as<smtk::model::Model>().session() :
    parent.owningModel().session();
  smtk::shared_ptr<Session> sess = smtk::dynamic_pointer_cast<Session>(sref.session());
  if (sess)
    {
    EntityHandle eh = sess->toEntity(parent);
    if (eh.isValid())
      {
      this->addUUIDsRecursive(sess, eh.object<vtkDataObject>(), uuids);
      }
    }
}

void SessionIOJSON::addUUIDsRecursive(smtk::shared_ptr<Session> s, vtkDataObject* node, common::UUIDArray& uuids)
{
  if (!node || !s)
    {
    return;
    }
  smtk::common::UUID uid = s->uuidOfHandleObject(node);
  uuids.push_back(uid);
  vtkMultiBlockDataSet* mbds = vtkMultiBlockDataSet::SafeDownCast(node);
  if (mbds)
    {
    int nc = mbds->GetNumberOfBlocks();
    for (int cc = 0; cc < nc; ++cc)
      {
      this->addUUIDsRecursive(s, mbds->GetBlock(cc), uuids);
      }
    }
}

    } // namespace exodus
  } // namespace bridge
} // namespace smtk
