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
#include "smtk/session/mesh/Resource.h"

#include "smtk/session/mesh/queries/BoundingBox.h"
#include "smtk/session/mesh/queries/ClosestPoint.h"
#include "smtk/session/mesh/queries/DistanceTo.h"
#include "smtk/session/mesh/queries/RandomPoint.h"

namespace smtk
{
namespace session
{
namespace mesh
{

namespace
{
typedef std::tuple<BoundingBox, ClosestPoint, DistanceTo, RandomPoint> QueryList;
}

Resource::Resource(const smtk::common::UUID& id, smtk::resource::Manager::Ptr manager)
  : smtk::resource::DerivedFrom<Resource, smtk::model::Resource>(id, manager)
{
  queries().registerQueries<QueryList>();
}

Resource::Resource(smtk::resource::Manager::Ptr manager)
  : smtk::resource::DerivedFrom<Resource, smtk::model::Resource>(manager)
{
  queries().registerQueries<QueryList>();
}

void Resource::setSession(const Session::Ptr& session)
{
  m_session = session->shared_from_this();
  this->registerSession(m_session);
}

smtk::mesh::ResourcePtr Resource::resource() const
{
  Topology* topology = m_session->topology(shared_from_this());
  return (topology != nullptr ? topology->m_resource : smtk::mesh::ResourcePtr());
}
} // namespace mesh
} // namespace session
} // namespace smtk
