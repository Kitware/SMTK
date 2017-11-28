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

#include "smtk/mesh/operators/RegisterOperations.h"
#include "smtk/mesh/resource/RegisterResources.h"

namespace
{
static bool registerToEnvironment()
{
  smtk::mesh::registerOperations(smtk::environment::OperationManager::instance());
  smtk::mesh::registerResources(smtk::environment::ResourceManager::instance());
  return true;
}
}

namespace smtk
{
namespace mesh
{
namespace environment
{
bool registered = registerToEnvironment();
}
}
}
