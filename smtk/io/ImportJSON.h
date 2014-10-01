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

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h" // For ManagerPtr

#include "smtk/common/UUID.h"

struct cJSON;

namespace smtk {
  namespace io {

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
  static int intoModel(const char* json, smtk::model::ManagerPtr manager);
  static int ofManager(cJSON* body, smtk::model::ManagerPtr manager);
  static int ofManagerEntity(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr manager);
  static int ofManagerArrangement(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr manager);
  static int ofManagerTessellation(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr manager);
  static int ofManagerFloatProperties(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr manager);
  static int ofManagerStringProperties(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr manager);
  static int ofManagerIntegerProperties(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr manager);
  static int ofRemoteBridgeSession(cJSON*, smtk::model::DefaultBridgePtr destBridge, smtk::model::ManagerPtr context);
  static int ofLocalBridgeSession(cJSON*, smtk::model::ManagerPtr context);
  static int ofOperator(cJSON* node, smtk::model::OperatorPtr& op, smtk::model::ManagerPtr context);
  static int ofOperatorResult(cJSON* node, smtk::model::OperatorResult& resOut, smtk::attribute::System* opSys);
  static int ofDanglingEntities(cJSON* node, smtk::model::ManagerPtr context);

  // Low-level helpers:
  static int getUUIDArrayFromJSON(cJSON* uidRec, std::vector<smtk::common::UUID>& uids);
  static int getStringArrayFromJSON(cJSON* arrayNode, std::vector<std::string>& text);
  static int getIntegerArrayFromJSON(cJSON* arrayNode, std::vector<long>& values);
  static int getRealArrayFromJSON(cJSON* arrayNode, std::vector<double>& values);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_io_ImportJSON_h
