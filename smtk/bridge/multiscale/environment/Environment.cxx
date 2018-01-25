//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/multiscale/environment/Exports.h"

#include "smtk/environment/Environment.h"

#include "smtk/bridge/multiscale/RegisterSession.h"

namespace
{
static bool registerToEnvironment()
{
  smtk::bridge::multiscale::registerOperations(smtk::environment::OperationManager::instance());
  smtk::bridge::multiscale::registerResources(smtk::environment::ResourceManager::instance());
  return true;
}
}

namespace smtk
{
namespace bridge
{
namespace multiscale
{
namespace environment
{
SMTKMULTISCALESESSIONENVIRONMENT_EXPORT bool registered = registerToEnvironment();
}
}
}
}
