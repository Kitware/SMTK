//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_remote_RemusRPCWorker_h
#define __smtk_session_remote_RemusRPCWorker_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "remus/worker/Job.h"
#include "remus/worker/Worker.h"

#include "smtk/model/StringData.h"

struct cJSON;

namespace smtk
{
namespace bridge
{
namespace remote
{

/**\brief A Remus worker that performs model operations using JSON-RPC.
  *
  * This class exists to wrap an SMTK model-manager into an
  * object that services JSON-RPC requests to perform operations.
  *
  * An instance of this class is tied to a Session
  * on the client side.
  *
  * Model synchronization is accomplished by serializing the
  * SMTK model into a JSON string maintained as field data on
  * an instance of this class.
  * Operators are also serialized (1) by this instance in order
  * for the client to enumerate them and (2) by the client in
  * order for this object to execute them.
  */
class RemusRPCWorker
{
public:
  smtkTypeMacroBase(RemusRPCWorker);
  smtkCreateMacro(RemusRPCWorker);
  virtual ~RemusRPCWorker();

  virtual void setOption(const std::string& optName, const std::string& optVal);
  virtual void clearOptions();

  void processJob(
    remus::worker::Worker*& w, remus::worker::Job& jd, remus::proto::JobRequirements& r);

  smtk::model::ManagerPtr manager();
  void setManager(smtk::model::ManagerPtr);

protected:
  RemusRPCWorker();

  void generateError(cJSON* err, const std::string& errMsg, const std::string& reqId);

  smtk::model::ManagerPtr m_modelMgr;
  smtk::model::StringData m_options;

private:
  RemusRPCWorker(const RemusRPCWorker&); // Not implemented.
  void operator=(const RemusRPCWorker&); // Not implemented.
};

} // namespace remote
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_remote_RemusRPCWorker_h
