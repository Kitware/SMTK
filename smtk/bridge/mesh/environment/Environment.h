//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_bridge_mesh_environment_Environment_h
#define __smtk_bridge_mesh_environment_Environment_h

#include "smtk/bridge/mesh/environment/Exports.h"

namespace smtk
{
namespace bridge
{
namespace mesh
{
namespace environment
{
struct SMTKMESHSESSIONENVIRONMENT_EXPORT RegisterToEnvironment
{
  RegisterToEnvironment();
  ~RegisterToEnvironment();

  RegisterToEnvironment(const RegisterToEnvironment&) = delete;
  RegisterToEnvironment& operator=(const RegisterToEnvironment&) = delete;
};

// Uses schwartz counter idiom for management
static RegisterToEnvironment registerToEnvironment;
}
}
}
}

#endif
