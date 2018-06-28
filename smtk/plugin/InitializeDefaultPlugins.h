//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_InitializeDefaultPlugins_h
#define __smtk_InitializeDefaultPlugins_h

#include "smtk/plugin/Exports.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
SMTKDEFAULTPLUGINS_EXPORT void initializeDefaultPlugins();
SMTKDEFAULTPLUGINS_EXPORT void loadDefaultPlugins();
}
}
}

#endif
