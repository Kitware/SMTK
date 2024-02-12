//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_Deprecation_h
#define smtk_common_Deprecation_h

#include "smtk/common/VersionMacros.h"

// The level at which warnings should be made.
#ifndef SMTK_DEPRECATION_LEVEL
// SMTK defaults to deprecation of its current version.
#define SMTK_DEPRECATION_LEVEL SMTK_VERSION_NUMBER
#endif

// API deprecated before 22.11 have already been removed.
#define SMTK_MINIMUM_DEPRECATION_LEVEL SMTK_VERSION_CHECK(22, 11, 0)

// Force the deprecation level to be at least that of SMTK's build
// configuration.
#if SMTK_DEPRECATION_LEVEL < SMTK_MINIMUM_DEPRECATION_LEVEL
#undef SMTK_DEPRECATION_LEVEL
#define SMTK_DEPRECATION_LEVEL SMTK_MINIMUM_DEPRECATION_LEVEL
#endif

#if 0 && __cplusplus >= 201402L
//  This is currently hard-disabled because compilers do not mix C++ attributes
//  and `__attribute__` extensions together well.
#define SMTK_DEPRECATION_II(reason) [[deprecated(reason)]]
#else
#if defined(_WIN32) || defined(_WIN64)
#define SMTK_DEPRECATION(reason) __declspec(deprecated(reason))
#elif defined(__clang__)
#if __has_extension(attribute_deprecated_with_message)
#define SMTK_DEPRECATION(reason) __attribute__((__deprecated__(reason)))
#else
#define SMTK_DEPRECATION(reason) __attribute__((__deprecated__))
#endif
#elif defined(__GNUC__)
#if (__GNUC__ >= 5) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 5))
#define SMTK_DEPRECATION(reason) __attribute__((__deprecated__(reason)))
#else
#define SMTK_DEPRECATION(reason) __attribute__((__deprecated__))
#endif
#else
#define SMTK_DEPRECATION(reason)
#endif
#endif

#define SMTK_DEPRECATION_REASON(version_major, version_minor, reason)                              \
  "SMTK Deprecated in " #version_major "." #version_minor ": " reason

#if SMTK_DEPRECATION_LEVEL >= SMTK_VERSION_CHECK(24, 01, 99)
#define SMTK_DEPRECATED_IN_NEXT(reason) SMTK_DEPRECATION(SMTK_DEPRECATION_REASON(24, NEXT, reason))
#else
#define SMTK_DEPRECATED_IN_NEXT(reason)
#endif

#if SMTK_DEPRECATION_LEVEL >= SMTK_VERSION_CHECK(23, 12, 99)
#define SMTK_DEPRECATED_IN_24_01(reason) SMTK_DEPRECATION(SMTK_DEPRECATION_REASON(24, 01, reason))
#else
#define SMTK_DEPRECATED_IN_24_01(reason)
#endif

#endif // smtk_common_Deprecation_h
