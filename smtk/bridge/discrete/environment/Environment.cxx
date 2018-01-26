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

#include "smtk/bridge/discrete/RegisterSession.h"

#include "smtk/bridge/discrete/environment/Exports.h"

namespace
{
bool registerToEnvironment()
{
  smtk::bridge::discrete::registerOperations(smtk::environment::OperationManager::instance());
  smtk::bridge::discrete::registerResources(smtk::environment::ResourceManager::instance());
  return true;
}
}

namespace smtk
{
namespace bridge
{
namespace discrete
{
namespace environment
{
SMTKDISCRETESESSIONENVIRONMENT_EXPORT bool registered = registerToEnvironment();
}
}
}
}
