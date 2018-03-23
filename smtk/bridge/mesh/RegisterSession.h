//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_mesh_RegisterSession_h
#define __smtk_session_mesh_RegisterSession_h

#include "smtk/bridge/mesh/Exports.h"
#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

namespace smtk
{
namespace bridge
{
namespace mesh
{

SMTKMESHSESSION_EXPORT void registerOperations(smtk::operation::Manager::Ptr&);
SMTKMESHSESSION_EXPORT void registerResources(smtk::resource::Manager::Ptr&);

SMTKMESHSESSION_EXPORT void unregisterOperations(smtk::operation::Manager::Ptr&);
SMTKMESHSESSION_EXPORT void unregisterResources(smtk::resource::Manager::Ptr&);
}
}
}

#endif
