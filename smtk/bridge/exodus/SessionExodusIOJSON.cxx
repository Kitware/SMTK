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

#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/io/ImportJSON.h"
#include "smtk/io/ExportJSON.h"

#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"

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
    path tryme(this->referencePath());
    tryme += absURL;
    if (exists(tryme))
      {
      absURL = tryme;
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
  (void)loadNativeModels;
  cJSON* preservedUUIDs = cJSON_GetObjectItem(sessionRec, "preservedUUIDs");
  cJSON* modelFiles = cJSON_GetObjectItem(sessionRec, "modelFiles");

  common::UUIDArray uids;
  if (preservedUUIDs)
    {
    smtk::io::ImportJSON::getUUIDArrayFromJSON(preservedUUIDs->child, uids);
    }

  if (loadNativeModels)
    {
    for (cJSON* entry = modelFiles->child; entry; entry = entry->next)
      {
      smtkDebugMacro(modelMgr->log(), "Loading file \"" << entry->valuestring << "\"");
      this->loadExodusFileWithUUIDs(model::SessionRef(modelMgr, session), entry->valuestring, uids);
      }
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
      uuidArray.push_back(mit->entity());
      this->addChildrenUUIDs(*mit, uuidArray);
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
        modelFiles.insert(url.string());
        }
      }
    }

  cJSON_AddItemToObject(sessionRec, "preservedUUIDs",
    smtk::io::ExportJSON::createUUIDArray(uuidArray));
  cJSON_AddItemToObject(sessionRec, "toplevelOffsets",
    smtk::io::ExportJSON::createIntegerArray(toplevelOffsets));
  cJSON_AddItemToObject(sessionRec, "modelNumbers",
    smtk::io::ExportJSON::createIntegerArray(modelNumbers));
  std::vector<std::string> urlArray(modelFiles.begin(), modelFiles.end());
  cJSON_AddItemToObject(sessionRec, "modelFiles",
    smtk::io::ExportJSON::createStringArray(urlArray));

  return 1;
}
// -- 3 --

/**\brief Add UUIDs of children to \a uuids array.
  *
  * The children are assumed to be listed in a stable order across file loads.
  */
void SessionIOJSON::addChildrenUUIDs(const model::EntityRef& parent, common::UUIDArray& uuids)
{
  model::EntityRefArray children;
  if (parent.isModel())
    {
    children = parent.as<model::Model>().submodelsAs<model::EntityRefArray>();
    this->addChildrenUUIDsIn(children, uuids);

    children = parent.as<model::Model>().groupsAs<model::EntityRefArray>();
    this->addChildrenUUIDsIn(children, uuids);

    // NB: Exodus session doesn't provide cells, but if it did, traverse them here.
    }
  else if (parent.isGroup())
    {
    children = parent.as<model::Group>().members<model::EntityRefArray>();
    this->addChildrenUUIDsIn(children, uuids);
    }
}

    } // namespace exodus
  } // namespace bridge
} // namespace smtk
