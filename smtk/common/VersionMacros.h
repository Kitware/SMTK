//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_VersionMacros_h
#define smtk_common_VersionMacros_h

#include "smtk/common/Version.h"

#define SMTK_VERSION_CHECK(major, minor, patch) (100000ULL * (major) + 1000UL * (minor) + (patch))

#define SMTK_VERSION_NUMBER                                                                        \
  SMTK_VERSION_CHECK(SMTK_VERSION_MAJOR, SMTK_VERSION_MINOR_INT, SMTK_VERSION_PATCH)

#endif // smtk_common_VersionMacros_h
