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
#include "smtk/resource/query/Manager.h"

#include "smtk/plugin/Manager.h"

namespace smtk
{
namespace resource
{
void Registrar::registerTo(const smtk::common::Managers::Ptr& managers)
{
  managers->insert(smtk::resource::Manager::create());
  auto resourceManager = managers->get<smtk::resource::Manager::Ptr>();
  smtk::plugin::Manager::instance()->registerPluginsTo(resourceManager);

  managers->insert(
    smtk::resource::query::Manager::create(managers->get<smtk::resource::Manager::Ptr>()));
  smtk::plugin::Manager::instance()->registerPluginsTo(
    managers->get<smtk::resource::query::Manager::Ptr>());

  auto& typeLabels = resourceManager->objectTypeLabels();
  typeLabels[smtk::common::typeName<smtk::resource::Component>()] = "component";
  typeLabels[smtk::common::typeName<smtk::resource::Resource>()] = "resource";
  typeLabels[smtk::common::typeName<smtk::resource::PersistentObject>()] = "object";
  smtk::string::Token dummy1("smtk::resource::Resource");
  smtk::string::Token dummy2("smtk::resource::Component");
  smtk::string::Token dummy3("smtk::resource::PersistentObject");
}

void Registrar::unregisterFrom(const smtk::common::Managers::Ptr& managers)
{
  managers->erase<smtk::resource::Manager::Ptr>();
}
} // namespace resource
} // namespace smtk
