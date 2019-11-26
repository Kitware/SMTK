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
#include "smtk/session/vtk/Resource.h"

namespace smtk
{
namespace session
{
namespace vtk
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
}
}
}
