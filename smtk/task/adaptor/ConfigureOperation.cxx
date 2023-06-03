//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/task/adaptor/ConfigureOperation.h"

#include "smtk/attribute/Resource.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace task
{
namespace adaptor
{

ConfigureOperation::ConfigureOperation() = default;
ConfigureOperation::ConfigureOperation(const Configuration& config)
{
  this->configureSelf(config);
}

ConfigureOperation::ConfigureOperation(const Configuration& config, Task* from, Task* to)
  : Superclass(config, from, to)
{
  this->configureSelf(config);
}

bool ConfigureOperation::reconfigureTask()
{
  return false;
}

void ConfigureOperation::configureSelf(const Configuration& config) {}

} // namespace adaptor
} // namespace task
} // namespace smtk
