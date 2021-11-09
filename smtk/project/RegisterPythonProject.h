//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_project_RegisterPythonProject_h
#define smtk_project_RegisterPythonProject_h

#include "smtk/CoreExports.h"

#include "smtk/project/Manager.h"

namespace smtk
{
namespace project
{

SMTKCORE_EXPORT bool registerPythonProject(
  const smtk::project::Manager::Ptr& projectManager,
  const std::string& moduleName);
}
} // namespace smtk

#endif
