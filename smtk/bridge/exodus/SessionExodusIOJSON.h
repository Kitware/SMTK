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

namespace smtk {
  namespace bridge {
    namespace exodus {

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
  virtual ~SessionIOJSON() { }

  virtual int importJSON(model::ManagerPtr modelMgr, const model::SessionPtr& session,
                         cJSON* sessionRec, bool loadNativeModels = false);
  virtual int exportJSON(model::ManagerPtr modelMgr, const model::SessionPtr& sessPtr,
                         cJSON* sessionRec, bool writeNativeModels = false);
  virtual int exportJSON(model::ManagerPtr modelMgr, const model::SessionPtr& session,
                         const common::UUIDs &modelIds, cJSON* sessionRec,
                         bool writeNativeModels = false);

protected:
  void addChildrenUUIDs(const model::EntityRef& parent, common::UUIDArray& uuids);

  template<typename T>
  void addChildrenUUIDsIn(const T& container, common::UUIDArray& uuids)
    {
    typename T::const_iterator it;
    for (it = container.begin(); it != container.end(); ++it)
      {
      uuids.push_back(it->entity());
      this->addChildrenUUIDs(*it, uuids);
      }
    }

  int loadExodusFileWithUUIDs(
    const model::SessionRef& sref,
    const std::string& url,
    const common::UUIDArray& preservedUUIDs);
};
// -- 1 --

    } // namespace exodus
  } // namespace bridge
} // namespace smtk

#endif // __smtk_model_SessionExodusIOJSON_h
