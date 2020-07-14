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
#include "smtk/resource/Registrar.h"

#include "smtk/resource/Manager.h"

#include "smtk/plugin/Manager.h"

namespace smtk
{
namespace resource
{
void Registrar::registerTo(const smtk::common::Managers::Ptr& managers)
{
  managers->insert(smtk::resource::Manager::create());
  smtk::plugin::Manager::instance()->registerPluginsTo(
    managers->get<smtk::resource::Manager::Ptr>());
}

void Registrar::unregisterFrom(const smtk::common::Managers::Ptr& managers)
{
  managers->erase<smtk::resource::Manager::Ptr>();
}
}
}
