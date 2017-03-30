//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_SessionIOJSON_h
#define __smtk_model_SessionIOJSON_h

#include "smtk/model/SessionIO.h"

struct cJSON;

namespace smtk
{
namespace model
{

/**\brief A base class for delegating session I/O to/from JSON.
  *
  * Subclasses should implement both
  * importJSON and exportJSON methods.
  */
class SMTKCORE_EXPORT SessionIOJSON : public SessionIO
{
public:
  smtkTypeMacro(SessionIOJSON);
  smtkCreateMacro(SessionIOJSON);

  virtual ~SessionIOJSON() {}

  virtual int importJSON(ManagerPtr modelMgr, const SessionPtr& session, cJSON* sessionRec,
    bool loadNativeModels = false);
  virtual int exportJSON(ManagerPtr modelMgr, const SessionPtr& sessPtr, cJSON* sessionRec,
    bool writeNativeModels = false);
  virtual int exportJSON(ManagerPtr modelMgr, const SessionPtr& session,
    const common::UUIDs& modelIds, cJSON* sessionRec, bool writeNativeModels = false);

protected:
  virtual int writeNativeModel(smtk::model::ManagerPtr modelMgr,
    const smtk::model::SessionPtr& sess, const smtk::model::Model& model,
    std::string& outNativeFile);
  virtual int loadNativeModel(smtk::model::ManagerPtr modelMgr, const smtk::model::SessionPtr& sess,
    const std::string& inNativeFile, std::string& loadedURL);
  virtual std::string getOutputFileNameForNativeModel(smtk::model::ManagerPtr modelMgr,
    const smtk::model::SessionPtr& sess, const smtk::model::Model& model) const;

  virtual int loadModelsRecord(smtk::model::ManagerPtr modelMgr, cJSON* sessionRec);
  virtual int loadMeshesRecord(smtk::model::ManagerPtr modelMgr, cJSON* sessionRec);
};

} // namespace model
} // namespace smtk

#endif // __smtk_model_SessionIOJSON_h
