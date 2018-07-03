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
#include "smtk/attribute/Registrar.h"

#include "smtk/attribute/Resource.h"

namespace smtk
{
namespace attribute
{
void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::attribute::Resource>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::attribute::Resource>();
}
}
}
