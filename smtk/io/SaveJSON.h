//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_io_SaveJSON_h
#define __smtk_io_SaveJSON_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/SystemConfig.h"

#include "smtk/common/UUID.h"

#include "smtk/model/EntityIterator.h" // For IteratorStyle
#include "smtk/model/Manager.h"        // For UUIDWithEntity

#include "cJSON.h"

namespace smtk
{
namespace io
{

class Logger;

/**\brief Indicate what type of data should be exported to JSON.
  *
  */
enum JSONFlags
{
  JSON_NOTHING = 0x00,    //!< Export nothing.
  JSON_ENTITIES = 0x01,   //!< Export model-entity entries in Manager
                          //!< (not including tessellations or properties).
  JSON_SESSIONS = 0x02,   //!< Export sessions (i.e., session IDs, the session type, operators).
  JSON_PROPERTIES = 0x04, //!< Export string/float/integer properties of
                          //!< model-entity entries in the Manager.

  JSON_TESSELLATIONS = 0x10, //!< Export tessellations of model-entity entries in the Manager.
  JSON_ANALYSISMESH = 0x20,  //!< Export tessellations of model-entity entries in the Manager.
  JSON_MESHES = 0x40,        //!< Export smtk::mesh of model-entity entries in the Manager.

  JSON_CLIENT_DATA = 0x07, //!< Export everything but tessellation data to clients.
  JSON_DEFAULT = 0xff      //!< By default, export everything.
};

/**\brief Save an SMTK model into a JSON-formatted string.
  *
  * Methods are also provided for creating cJSON nodes representing
  * individual records and groups of records from SMTK storage (a model
  * manager).
  * These may be used to provide concise answers to specific queries
  * and avoid storing or transmitting a potentially-large string.
  */
class SMTKCORE_EXPORT SaveJSON
{
public:
  static cJSON* fromUUIDs(const smtk::common::UUIDs& uids);

  static int fromModelManager(
    cJSON* json, smtk::model::ManagerPtr modelMgr, JSONFlags sections = JSON_DEFAULT);
  static std::string fromModelManager(
    smtk::model::ManagerPtr modelMgr, JSONFlags sections = JSON_DEFAULT);
  static bool fromModelManagerToFile(smtk::model::ManagerPtr modelMgr, const char* filename);

  // Serialize a Set (for now, only smtk::model::StoredModel entries are handled). For debug use only.
  static int fromSet(cJSON* pnode, smtk::resource::SetPtr& rset);

  /// Returns true if all models have URLs; false otherwise.
  static bool canSaveModels(const smtk::model::Models& modelsToSave);
  /**\brief Store state changes required to save in a mode into the given object \a obj .
    *
    * Returns true when saving in the given \a mode is possible and false otherwise.
    *
    */
  template <typename T>
  static bool prepareToSave(const smtk::model::Models& modelsToSave,
    const std::string& mode,     // "save", "save as", or "save a copy"
    const std::string& filename, // only used when mode == "save as" or "save a copy"
    const std::string& renamePolicy, bool embedData,
    T& obj // structure whose ivars will contain changes to be made before/during/after saving.
    );
  /// Save \a models (and potentially others that share the same URLs) to their pre-existing URLs.
  static int save(cJSON* pnode, const smtk::model::Models& models, bool renameModels = true,
    const std::string& embedDir = "");

  template <typename T>
  static int forEntities(cJSON* json, const T& entities,
    smtk::model::IteratorStyle relatedEntities = smtk::model::ITERATE_MODELS,
    JSONFlags sections = JSON_DEFAULT);
  template <typename T>
  static std::string forEntities(const T& entities,
    smtk::model::IteratorStyle relatedEntities = smtk::model::ITERATE_MODELS,
    JSONFlags sections = JSON_DEFAULT);

  static int forManager(cJSON* body, cJSON* sess, cJSON* mesh, smtk::model::ManagerPtr modelMgr,
    JSONFlags sections = JSON_DEFAULT);
  static int forManagerEntity(
    smtk::model::UUIDWithEntityPtr& entry, cJSON*, smtk::model::ManagerPtr modelMgr);
  static int forManagerTessellation(
    const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr modelMgr);
  static int forManagerAnalysis(
    const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr modelMgr);
  static int forManagerFloatProperties(
    const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr modelMgr);
  static int forManagerStringProperties(
    const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr modelMgr);
  static int forManagerIntegerProperties(
    const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr modelMgr);
  static int forManagerMeshes(
    smtk::mesh::ManagerPtr meshes, cJSON*, smtk::model::ManagerPtr modelMgr);
  static int forManagerSession(const smtk::common::UUID& sessionId, cJSON*,
    smtk::model::ManagerPtr modelMgr, bool writeNativeModels = false,
    const std::string& referencePath = std::string());
  static int forManagerSessionPartial(const smtk::common::UUID& sessionId,
    const common::UUIDs& modelIds, cJSON*, smtk::model::ManagerPtr modelMgrId,
    bool writeNativeModels = false, const std::string& referencePath = std::string());
  //static int forModelOperations(const smtk::common::UUID& uid, cJSON*, smtk::model::ManagerPtr modelMgr);
  // static int forOperationDefinitions(smtk::attribute::CollectionPtr opSys, cJSON*);
  // static int forOperation(smtk::operation::OperationSpecification op, cJSON*);
  // static int forOperation(smtk::operation::OperationPtr op, cJSON*);
  // static int forOperationResult(Result res, cJSON*);
  static int forDanglingEntities(
    const smtk::common::UUID& sessionId, cJSON* node, smtk::model::ManagerPtr modelMgr);

  // Utilities used by above:
  static int addModelsRecord(
    const smtk::model::ManagerPtr modelMgr, const smtk::common::UUIDs& modelIds, cJSON* sessionRec);
  static int addModelsRecord(
    const smtk::model::ManagerPtr modelMgr, const smtk::model::Models& models, cJSON* sessionRec);
  static int addMeshesRecord(
    const smtk::model::ManagerPtr modelMgr, const smtk::common::UUIDs& modelIds, cJSON* sessionRec);
  static int addMeshesRecord(
    const smtk::model::ManagerPtr modelMgr, const smtk::model::Models& inModels, cJSON* sessionRec);

  static int forModelWorker(cJSON* workerDescription, const std::string& meshTypeIn,
    const std::string& meshTypeOut, smtk::model::SessionPtr session, const std::string& engine,
    const std::string& site, const std::string& root, const std::string& workerPath,
    const std::string& requirementsFileName);

  //write out a all the information about a single mesh collection
  static int forSingleCollection(cJSON* mdesc, smtk::mesh::CollectionPtr collection);

  // Serialize all the input mesh Collections in mesh manager \a meshMgr,
  // given the mesh \a collectionIds.
  static int forMeshCollections(
    cJSON* pnode, const smtk::common::UUIDs& collectionIds, smtk::mesh::ManagerPtr meshMgr);

  // Serialize all the smtk::mesh collections associated with given \a modelid.
  static int forModelMeshes(
    const smtk::common::UUID& modelid, cJSON* pnode, smtk::model::ManagerPtr modelMgr);

  static int forLog(cJSON* logrecordarray, const smtk::io::Logger& log, std::size_t start = 0,
    std::size_t end = static_cast<std::size_t>(-1));

  // JSON-RPC helpers:
  static cJSON* createRPCRequest(
    const std::string& method, const std::string& params, const std::string& reqId);
  static cJSON* createRPCRequest(const std::string& method, cJSON*& params,
    const std::string& reqId, int paramsType = cJSON_Array);

  // Low-level helpers:
  static cJSON* createStringArray(const std::vector<std::string>& arr);
  static cJSON* createUUIDArray(const std::vector<smtk::common::UUID>& arr);
  static cJSON* createIntegerArray(const std::vector<long>& arr);
  static int forFloatData(cJSON* dict, const smtk::model::FloatData& fdata);
  static int forStringData(cJSON* dict, const smtk::model::StringData& sdata);
  static int forIntegerData(cJSON* dict, const smtk::model::IntegerData& idata);
};

} // namespace model
} // namespace smtk

#endif // __smtk_io_SaveJSON_h
