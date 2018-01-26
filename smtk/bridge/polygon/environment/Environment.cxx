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

#include "smtk/bridge/polygon/RegisterSession.h"

#include "smtk/bridge/polygon/environment/Exports.h"

namespace
{
bool registerToEnvironment()
{
  smtk::bridge::polygon::registerOperations(smtk::environment::OperationManager::instance());
  smtk::bridge::polygon::registerResources(smtk::environment::ResourceManager::instance());
  return true;
}
}

namespace smtk
{
namespace bridge
{
namespace polygon
{
namespace environment
{
SMTKPOLYGONSESSIONENVIRONMENT_EXPORT bool registered = registerToEnvironment();
}
}
}
}
