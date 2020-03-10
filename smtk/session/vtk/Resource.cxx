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

#include "smtk/extension/vtk/geometry/BoundingBox.h"
#include "smtk/extension/vtk/geometry/ClosestPoint.h"
#include "smtk/extension/vtk/geometry/DistanceTo.h"

namespace smtk
{
namespace session
{
namespace vtk
{

namespace
{
typedef std::tuple<smtk::extension::vtk::geometry::BoundingBox,
  smtk::extension::vtk::geometry::ClosestPoint, smtk::extension::vtk::geometry::DistanceTo>
  QueryList;
}

Resource::Resource(const smtk::common::UUID& id, smtk::resource::ManagerPtr manager)
  : smtk::resource::DerivedFrom<Resource, smtk::model::Resource>(id, manager)
{
  queries().registerQueries<QueryList>();
}

Resource::Resource(smtk::resource::ManagerPtr manager)
  : smtk::resource::DerivedFrom<Resource, smtk::model::Resource>(manager)
{
  queries().registerQueries<QueryList>();
}

void Resource::setSession(const Session::Ptr& session)
{
  m_session = session->shared_from_this();
  this->registerSession(m_session);
}
}
}
}
