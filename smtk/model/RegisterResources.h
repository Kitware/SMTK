//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_RegisterResources_h
#define __smtk_model_RegisterResources_h

#include "smtk/CoreExports.h"

#include "smtk/resource/Manager.h"

namespace smtk
{
namespace model
{

SMTKCORE_EXPORT void registerResources(smtk::resource::Manager::Ptr&);
SMTKCORE_EXPORT void unregisterResources(smtk::resource::Manager::Ptr&);
}
}

#endif
