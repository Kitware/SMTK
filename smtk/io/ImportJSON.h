//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_io_ImportJSON_h
#define __smtk_io_ImportJSON_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h" // For ManagerPtr

#include "smtk/common/UUID.h"

#include "smtk/model/StringData.h"

struct cJSON;

namespace smtk {
  namespace io {

class Logger;

/**\brief Import an SMTK model from JSON data.
  *
  * Methods are also provided for importing individual records
  * and groups of records directly from cJSON nodes.
  * These may be used to update relevant entities without
  * storing or transmitting a potentially-large string.
  */
class SMTKCORE_EXPORT ImportJSON
{
public:
  static int intoModelManager(const char* json, smtk::model::ManagerPtr manager);
  static int ofManager(cJSON* body, smtk::model::ManagerPtr manager);
  static int ofManagerEntity(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr manager);
  static int ofManagerArrangement(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr manager);
  static int ofManagerTessellation(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr manager);
  static int ofManagerAnalysis(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr manager);
  static int ofManagerFloatProperties(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr manager);
  static int ofManagerStringProperties(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr manager);
  static int ofManagerIntegerProperties(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr manager);
  static int forManagerMeshes(smtk::mesh::ManagerPtr meshes, cJSON*, smtk::model::ManagerPtr modelMgr);
  static int ofRemoteSession(cJSON*, smtk::model::DefaultSessionPtr destSession, smtk::model::ManagerPtr context);
  static int ofLocalSession(cJSON*, smtk::model::ManagerPtr context);
  static int ofOperator(cJSON* node, smtk::model::OperatorPtr& op, smtk::model::ManagerPtr context);
  static int ofOperatorResult(cJSON* node, smtk::model::OperatorResult& resOut, smtk::model::RemoteOperatorPtr op);
  static int ofDanglingEntities(cJSON* node, smtk::model::ManagerPtr context);

  static int ofLog(const char* jsonStr, smtk::io::Logger& log);
  static int ofLog(cJSON* logrecordarray, smtk::io::Logger& log);

  //write all mesh collections that have associations to a model
  static int ofMeshesOfModel(cJSON* node, smtk::model::ManagerPtr modelMgr, bool updateExisting=false);
  //write all mesh properties for the collection
  static int ofMeshProperties(cJSON* node, smtk::mesh::CollectionPtr collection);
  // Mid-level helpers:
  static std::string sessionNameFromTagData(cJSON* tagData);
  static smtk::model::StringList sessionFileTypesFromTagData(cJSON* tagData);

  // Low-level helpers:
  static int getUUIDArrayFromJSON(cJSON* uidRec, std::vector<smtk::common::UUID>& uids);
  static int getStringArrayFromJSON(cJSON* arrayNode, std::vector<std::string>& text);
  static int getIntegerArrayFromJSON(cJSON* arrayNode, std::vector<long>& values);
  static int getRealArrayFromJSON(cJSON* arrayNode, std::vector<double>& values);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_io_ImportJSON_h
