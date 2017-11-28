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
#include "smtk/attribute/RegisterResources.h"

#include "smtk/attribute/Collection.h"

namespace smtk
{
namespace attribute
{

void registerResources(smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::attribute::Collection>();
}
}
}
