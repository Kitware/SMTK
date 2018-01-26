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
#include "smtk/environment/Exports.h"

namespace
{
bool linkManagers()
{
  smtk::environment::OperationManager::instance()->registerResourceManager(
    smtk::environment::ResourceManager::instance());
  return true;
}
}

namespace smtk
{
namespace environment
{
SMTKENVIRONMENT_EXPORT bool registered = linkManagers();
}
}
