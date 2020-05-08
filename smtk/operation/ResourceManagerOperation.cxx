//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/ResourceManagerOperation.h"

#include "smtk/resource/Manager.h"

namespace smtk
{
namespace operation
{

void ResourceManagerOperation::setResourceManager(smtk::resource::WeakManagerPtr managerPtr)
{
  m_resourceManager = managerPtr;
}

smtk::resource::ManagerPtr ResourceManagerOperation::resourceManager()
{
  return m_resourceManager.lock();
}
}
}
