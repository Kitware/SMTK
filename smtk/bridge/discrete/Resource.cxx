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
#include "smtk/bridge/discrete/Resource.h"

namespace smtk
{
namespace bridge
{
namespace discrete
{

Resource::Resource(const smtk::common::UUID& id, resource::Manager::Ptr manager)
  : smtk::resource::DerivedFrom<Resource, smtk::model::Manager>(id, manager)
{
}

Resource::Resource(resource::Manager::Ptr manager)
  : smtk::resource::DerivedFrom<Resource, smtk::model::Manager>(manager)
{
}

void Resource::setSession(const Session::Ptr& session)
{
  m_session = session->shared_from_this();
  this->registerSession(m_session);
}

/// Return a shared pointer to the session backing a discrete operator.
SessionPtr Resource::discreteSession()
{
  return m_session;
}

ConstSessionPtr Resource::discreteSession() const
{
  return m_session;
}

vtkModelItem* Resource::discreteEntity(const smtk::model::EntityRef& smtkEntity)
{
  return this->discreteSession()->entityForUUID(smtkEntity.entity());
}
}
}
}
