//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_environment_Environment_h
#define __smtk_environment_Environment_h

#include "smtk/environment/Exports.h"

#include "smtk/common/Singleton.h"
#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

#ifndef smtkEnvironment_EXPORTS
extern
#endif
  template class SMTKENVIRONMENT_EXPORT smtk::common::Singleton<smtk::operation::Manager>;

#ifndef smtkEnvironment_EXPORTS
extern
#endif
  template class SMTKENVIRONMENT_EXPORT smtk::common::Singleton<smtk::resource::Manager>;

namespace smtk
{
namespace environment
{
typedef smtk::common::Singleton<smtk::operation::Manager> OperationManager;
typedef smtk::common::Singleton<smtk::resource::Manager> ResourceManager;
}
}

#endif
