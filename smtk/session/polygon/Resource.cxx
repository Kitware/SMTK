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
#include "smtk/session/polygon/Resource.h"

namespace smtk
{
namespace session
{
namespace polygon
{

Resource::Resource(const smtk::common::UUID& id, smtk::resource::ManagerPtr manager)
  : smtk::resource::DerivedFrom<Resource, smtk::model::Resource>(id, manager)
{
}

Resource::Resource(smtk::resource::ManagerPtr manager)
  : smtk::resource::DerivedFrom<Resource, smtk::model::Resource>(manager)
{
}

void Resource::setSession(const Session::Ptr& session)
{
  m_session = session->shared_from_this();
  this->registerSession(m_session);
}

/// Return a shared pointer to the session backing a polygon operator.
SessionPtr Resource::polygonSession()
{
  return m_session;
}

ConstSessionPtr Resource::polygonSession() const
{
  return m_session;
}

void Resource::addStorage(
  const smtk::common::UUID& uid, smtk::session::polygon::internal::entity::Ptr storage)
{
  this->polygonSession()->addStorage(uid, storage);
}

bool Resource::removeStorage(const smtk::common::UUID& uid)
{
  return this->polygonSession()->removeStorage(uid);
}
}
}
}
