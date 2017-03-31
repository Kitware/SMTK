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
#ifndef SHIBOKEN_SKIP
#include "smtk/bridge/remote/RemusStaticSessionInfo.h"

#include "smtk/bridge/remote/RemusConnection.h"
#include "smtk/bridge/remote/Session.h"

#include "smtk/io/LoadJSON.h"

#include "remus/proto/JobRequirements.h"

#include "cJSON.h"

using namespace smtk::common;
using namespace smtk::model;
using namespace smtk::io;

namespace smtk {
  namespace bridge {
    namespace remote {

/// Default constructor. This only exists so that the class can be used in STL containers.
RemusStaticSessionInfo::RemusStaticSessionInfo()
{
}

/// Create session information, initialized with job requirements and a connection from Remus.
RemusStaticSessionInfo::RemusStaticSessionInfo(
  RemusConnectionPtr conn,
  const remus::proto::JobRequirements& jobReq,
  const std::string& meshType
)
  : m_conn(conn), m_meshType(meshType), m_tags(jobReq.tag())
{
  std::string btag = jobReq.tag();
  cJSON* info = NULL;
  if (!btag.empty())
    {
    info = cJSON_Parse(btag.c_str());
    // Add the server information to the tag
    }
  if (!info)
    {
    info = cJSON_CreateObject();
    }
  cJSON_AddItemToObject(
    info, "server", cJSON_CreateString(this->m_conn->connection().endpoint().c_str()));
  this->m_name = !jobReq.workerName().empty() ?
    jobReq.workerName() :
    LoadJSON::sessionNameFromTagData(info);
  //StringList fileTypes = LoadJSON::sessionFileTypesFromTagData(info);
  cJSON_Delete(info);
  if (jobReq.hasRequirements())
    this->m_operatorXML = jobReq.requirements();
}

/// Copy constructor for session information.
RemusStaticSessionInfo::RemusStaticSessionInfo(
  const RemusStaticSessionInfo& other)
: m_conn(other.m_conn), m_meshType(other.m_meshType),
  m_name(other.m_name), m_tags(other.m_tags),
  m_operatorXML(other.m_operatorXML)
{
}

/**\brief Pass on remotely-provided pre-construction configuration options.
  *
  * \sa SessionStaticSetup
  */
int RemusStaticSessionInfo::staticSetup(
  const std::string& optName,
  const smtk::model::StringList& optVal)
{
  return this->m_conn->staticSetup(this->m_name, optName, optVal);
}

/// Construct a Session plus a session of the given type at the remote end.
smtk::model::SessionPtr RemusStaticSessionInfo::operator () () const
{
  smtk::common::UUID sessId = this->m_conn->beginSession(this->m_name);
  // FIXME: Set the default engine? Any other setup?
  smtk::model::Session::Ptr sess = this->m_conn->findSession(sessId);
  return sess;
}

    } // namespace remote
 } // namespace bridge
} // namespace smtk
#endif // SHIBOKEN_SKIP
