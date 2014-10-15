//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_io_ExportJSON_h
#define __smtk_io_ExportJSON_h

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/SystemConfig.h"

#include "smtk/common/UUID.h"

#include "smtk/model/Manager.h" // For UUIDWithEntity

struct cJSON;

namespace smtk {
  namespace io {

/**\brief Indicate what data should be exported to JSON.
  *
  */
enum JSONFlags
{
  JSON_NOTHING       = 0x00, //!< Export nothing.
  JSON_ENTITIES      = 0x01, //!< Export model-entity entries in Manager (not including tessellations or properties).
  JSON_BRIDGES       = 0x02, //!< Export bridge sessions (i.e., bridge session IDs, the bridge type, and operators).
  JSON_TESSELLATIONS = 0x04, //!< Export tessellations of model-entity entries in the Manager.
  JSON_PROPERTIES    = 0x08, //!< Export string/float/integer properties of model-entity entries in the Manager.
  JSON_DEFAULT       = 0xff  //!< By default, export everything.
};

/**\brief Export an SMTK model into a JSON-formatted string.
  *
  * Methods are also provided for creating cJSON nodes representing
  * individual records and groups of records from SMTK storage (a model
  * manager).
  * These may be used to provide concise answers to specific queries
  * and avoid storing or transmitting a potentially-large string.
  */
class SMTKCORE_EXPORT ExportJSON
{
public:
  static cJSON* fromUUIDs(const smtk::common::UUIDs& uids);

  static int fromModel(cJSON* json, smtk::model::ManagerPtr modelMgr, JSONFlags sections = JSON_DEFAULT);
  static std::string fromModel(smtk::model::ManagerPtr modelMgr, JSONFlags sections = JSON_DEFAULT);

  static int forManager(cJSON* body, cJSON* sess, smtk::model::ManagerPtr modelMgr, JSONFlags sections = JSON_DEFAULT);
  static int forManagerEntity(smtk::model::UUIDWithEntity& entry, cJSON*, smtk::model::ManagerPtr modelMgr);
  static int forManagerArrangement(const smtk::model::UUIDWithArrangementDictionary& entry, cJSON*, smtk::model::ManagerPtr modelMgr);
  static int forManagerTessellation(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr modelMgr);
  static int forManagerFloatProperties(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr modelMgr);
  static int forManagerStringProperties(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr modelMgr);
  static int forManagerIntegerProperties(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr modelMgr);
  static int forManagerBridgeSession(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr modelMgr);
  //static int forModelOperators(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr modelMgr);
  static int forOperatorDefinitions(smtk::attribute::System* opSys, cJSON*);
  static int forOperator(smtk::model::OperatorSpecification op, cJSON*);
  static int forOperator(smtk::model::OperatorPtr op, cJSON*);
  static int forOperatorResult(smtk::model::OperatorResult res, cJSON*);
  static int forDanglingEntities(const smtk::common::UUID& bridgeSessionId, cJSON* node, smtk::model::ManagerPtr modelMgr);

  static int forModelWorker(
    cJSON* workerDescription,
    const std::string& meshTypeIn, const std::string& meshTypeOut,
    const std::string& kernel, const std::string& engine,
    const std::string& site, const std::string& root,
    const std::string& workerPath, const std::string& requirementsFileName);

  // Low-level helpers:
  static cJSON* createStringArray(std::vector<std::string>& arr);
  static cJSON* createUUIDArray(std::vector<smtk::common::UUID>& arr);
  static cJSON* createIntegerArray(std::vector<long>& arr);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_io_ExportJSON_h
