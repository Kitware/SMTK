//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_mesh_Resource_h
#define __smtk_session_mesh_Resource_h

#include "smtk/bridge/mesh/Exports.h"
#include "smtk/bridge/mesh/Session.h"

#include "smtk/model/Manager.h"

#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Manager.h"

namespace smtk
{
namespace bridge
{
namespace mesh
{

class SMTKMESHSESSION_EXPORT Resource
  : public smtk::resource::DerivedFrom<Resource, smtk::model::Manager>
{
public:
  smtkTypeMacro(smtk::bridge::mesh::Resource);
  smtkSharedPtrCreateMacro(smtk::resource::Resource);

  virtual ~Resource() {}

  const Session::Ptr& session() const { return m_session; }
  void setSession(const Session::Ptr&);

protected:
  Resource(const smtk::common::UUID&, smtk::resource::Manager::Ptr manager = nullptr);
  Resource(smtk::resource::Manager::Ptr manager = nullptr);

  Session::Ptr m_session;
};

} // namespace mesh
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_mesh_Resource_h
