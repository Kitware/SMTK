//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_discrete_RegisterSession_h
#define __smtk_session_discrete_RegisterSession_h

#include "smtk/bridge/discrete/Exports.h"
#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

namespace smtk
{
namespace bridge
{
namespace discrete
{

SMTKDISCRETESESSION_EXPORT void registerOperations(smtk::operation::Manager::Ptr&);
SMTKDISCRETESESSION_EXPORT void registerResources(smtk::resource::Manager::Ptr&);
}
}
}

#endif
