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

#include "smtk/model/RegisterOperations.h"
#include "smtk/model/RegisterResources.h"

#include "smtk/model/environment/Exports.h"

namespace
{
bool registerToEnvironment()
{
  smtk::model::registerOperations(smtk::environment::OperationManager::instance());
  smtk::model::registerResources(smtk::environment::ResourceManager::instance());
  return true;
}
}

namespace smtk
{
namespace model
{
namespace environment
{
SMTKMODELENVIRONMENT_EXPORT bool registered = registerToEnvironment();
}
}
}
