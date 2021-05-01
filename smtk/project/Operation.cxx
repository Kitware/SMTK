//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/project/Operation.h"

#include "smtk/project/Manager.h"

namespace smtk
{
namespace project
{

void Operation::setProjectManager(smtk::project::WeakManagerPtr managerPtr)
{
  m_projectManager = managerPtr;
}

smtk::project::ManagerPtr Operation::projectManager()
{
  return m_projectManager.lock();
}
} // namespace project
} // namespace smtk
