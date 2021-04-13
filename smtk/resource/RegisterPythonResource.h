//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_resource_RegisterPythonResource_h
#define __smtk_resource_RegisterPythonResource_h

#include "smtk/CoreExports.h"

#include "smtk/resource/Manager.h"

namespace smtk
{
namespace resource
{

SMTKCORE_EXPORT bool registerPythonResource(
  const smtk::resource::Manager::Ptr& resourceManager,
  const std::string& moduleName);
}
} // namespace smtk

#endif
