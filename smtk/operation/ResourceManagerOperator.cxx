//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/ResourceManagerOperator.h"

#include "smtk/resource/Manager.h"

namespace smtk
{
namespace operation
{

void ResourceManagerOperator::setResourceManager(smtk::resource::WeakManagerPtr managerPtr)
{
  m_resourceManager = managerPtr;
}

smtk::resource::ManagerPtr ResourceManagerOperator::resourceManager()
{
  return m_resourceManager.lock();
}

bool ResourceManagerOperator::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  // To create a resource, we must have a resource manager that can read
  // resources.
  if (this->resourceManager() == nullptr)
  {
    return false;
  }

  return true;
}
}
}
