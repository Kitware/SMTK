//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#ifndef __smtk_session_remote_RemusStaticSessionInfo_h
#define __smtk_session_remote_RemusStaticSessionInfo_h
#ifndef SHIBOKEN_SKIP

#include "smtk/PublicPointerDefs.h"
#include "smtk/bridge/remote/Exports.h"

#include "smtk/model/StringData.h"

#include <string>

namespace remus
{
namespace proto
{
class JobRequirements;
}
}

namespace smtk
{
namespace bridge
{
namespace remote
{

/**\brief A helper class for the Remus remote session.
  *
  * Instances of this class can be passed as a SessionConstructor,
  * which is an smtk::function<SessionPtr()>.
  * In addition to simply creating a new session, they
  * hold metadata about how the session should be configured
  * and can invoke methods to prepare the session for use with
  * a specific Remus worker that has been registered.
  */
class SMTKREMOTESESSION_EXPORT RemusStaticSessionInfo
{
public:
  RemusStaticSessionInfo();
  RemusStaticSessionInfo(RemusConnectionPtr conn, const remus::proto::JobRequirements& jobReq,
    const std::string& meshType);
  RemusStaticSessionInfo(const RemusStaticSessionInfo&);

  int staticSetup(const std::string& optName, const smtk::model::StringList& optVal);

  smtk::model::SessionPtr operator()() const;

  std::string name() const { return this->m_name; }
  std::string tags() const { return this->m_tags; }

  RemusConnectionPtr m_conn;
  std::string m_meshType;
  std::string m_name;
  std::string m_tags;
  std::string m_operatorXML;
};

} // namespace remote
} // namespace bridge
} // namespace smtk

#endif // SHIBOKEN_SKIP
#endif // __smtk_session_remote_RemusStaticSessionInfo_h
