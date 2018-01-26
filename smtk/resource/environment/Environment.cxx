//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/environment/Environment.h"

#include "smtk/resource/RegisterOperations.h"
#include "smtk/resource/RegisterResources.h"
#include "smtk/resource/environment/Exports.h"

namespace
{
bool registerToEnvironment()
{
  smtk::resource::registerOperations(smtk::environment::OperationManager::instance());
  smtk::resource::registerResources(smtk::environment::ResourceManager::instance());
  return true;
}
}

namespace smtk
{
namespace resource
{
namespace environment
{
SMTKRESOURCEENVIRONMENT_EXPORT bool registered = registerToEnvironment();
}
}
}
