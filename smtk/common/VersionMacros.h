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

#define SMTK_VERSION_CHECK(major, minor) (100ULL * (major) + (minor))

#define SMTK_VERSION_NUMBER SMTK_VERSION_CHECK(SMTK_VERSION_MAJOR, SMTK_VERSION_MINOR_INT)

#endif // smtk_common_VersionMacros_h
