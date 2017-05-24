//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_SessionExodusIOJSON_h
#define __smtk_model_SessionExodusIOJSON_h

#include "smtk/bridge/exodus/Exports.h"

#include "smtk/model/SessionIOJSON.h"

struct cJSON;

class vtkDataObject;

namespace smtk
{
namespace bridge
{
namespace exodus
{

class Session;

/**\brief A base class for delegating session I/O to/from JSON.
  *
  * Subclasses should implement both
  * importJSON and exportJSON methods.
  */
// ++ 1 ++
class SMTKEXODUSSESSION_EXPORT SessionIOJSON : public smtk::model::SessionIOJSON
{
public:
  smtkTypeMacro(SessionIOJSON);
  smtkCreateMacro(SessionIOJSON);
  SessionIOJSON();
  virtual ~SessionIOJSON() {}

  virtual int saveJSON(
    cJSON* node, const smtk::model::SessionRef& sref, const smtk::model::Models& models) override;

  virtual int importJSON(model::ManagerPtr modelMgr, const model::SessionPtr& session,
    cJSON* sessionRec, bool loadNativeModels = false);
  virtual int exportJSON(model::ManagerPtr modelMgr, const model::SessionPtr& sessPtr,
    cJSON* sessionRec, bool writeNativeModels = false);
  virtual int exportJSON(model::ManagerPtr modelMgr, const model::SessionPtr& session,
    const common::UUIDs& modelIds, cJSON* sessionRec, bool writeNativeModels = false);

protected:
  void addModelUUIDs(const model::EntityRef& parent, common::UUIDArray& uuids);
  void addUUIDsRecursive(
    smtk::shared_ptr<Session> s, vtkDataObject* node, common::UUIDArray& uuids);

  int loadExodusFileWithUUIDs(
    const model::SessionRef& sref, const std::string& url, const common::UUIDArray& preservedUUIDs);
};
// -- 1 --

} // namespace exodus
} // namespace bridge
} // namespace smtk

#endif // __smtk_model_SessionExodusIOJSON_h
