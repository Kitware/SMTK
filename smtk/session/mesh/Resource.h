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

#include "smtk/session/mesh/Exports.h"
#include "smtk/session/mesh/Session.h"

#include "smtk/model/Resource.h"

#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Manager.h"

namespace smtk
{
namespace session
{
namespace mesh
{

class SMTKMESHSESSION_EXPORT Resource
  : public smtk::resource::DerivedFrom<Resource, smtk::model::Resource>
{
public:
  smtkTypeMacro(smtk::session::mesh::Resource);
  smtkSharedPtrCreateMacro(smtk::resource::Resource);

  virtual ~Resource() {}

  const Session::Ptr& session() const { return m_session; }
  void setSession(const Session::Ptr&);

  const smtk::mesh::CollectionPtr& collection() const;

protected:
  Resource(const smtk::common::UUID&, smtk::resource::Manager::Ptr manager = nullptr);
  Resource(smtk::resource::Manager::Ptr manager = nullptr);

  Session::Ptr m_session;
};

} // namespace mesh
} // namespace session
} // namespace smtk

#endif // __smtk_session_mesh_Resource_h
