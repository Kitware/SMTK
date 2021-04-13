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
#include "smtk/geometry/Registrar.h"

#include "smtk/geometry/Manager.h"
#include "smtk/resource/Manager.h"

#include "smtk/plugin/Manager.h"

namespace smtk
{
namespace geometry
{
void Registrar::registerTo(const smtk::common::Managers::Ptr& managers)
{
  managers->insert(smtk::geometry::Manager::create());

  if (managers->contains<smtk::resource::Manager>())
  {
    managers->get<smtk::geometry::Manager::Ptr>()->registerResourceManager(
      managers->get<smtk::resource::Manager::Ptr>());
  }
  smtk::plugin::Manager::instance()->registerPluginsTo(
    managers->get<smtk::geometry::Manager::Ptr>());
}

void Registrar::unregisterFrom(const smtk::common::Managers::Ptr& managers)
{
  managers->erase<smtk::geometry::Manager::Ptr>();
}
} // namespace geometry
} // namespace smtk
