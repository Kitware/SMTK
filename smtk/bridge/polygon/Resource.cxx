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
#include "smtk/bridge/polygon/Resource.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{

Resource::Resource(const smtk::common::UUID& id, resource::Manager::Ptr manager)
  : smtk::resource::DerivedFrom<Resource, smtk::model::Resource>(id, manager)
{
}

Resource::Resource(resource::Manager::Ptr manager)
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
  const smtk::common::UUID& uid, smtk::bridge::polygon::internal::entity::Ptr storage)
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
