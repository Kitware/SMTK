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
#include "smtk/model/RegisterResources.h"

#include "smtk/model/Manager.h"

namespace smtk
{
namespace model
{

void registerResources(smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::model::Manager>();
}
}
}
