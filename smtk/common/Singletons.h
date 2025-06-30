//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_Singletons_h
#define smtk_common_Singletons_h

#include "smtk/common/TypeContainer.h" // For API and exports.

namespace smtk
{
namespace common
{

/// Return a container of singleton objects indexed by their type.
///
/// Because the index is based on the type of the object being
/// contained (it is a checksum computed on the typename-string),
/// there can be only zero or one objects of a given type in the
/// container.
SMTKCORE_EXPORT TypeContainer& singletons();

/// Destroy the container holding all registered singleton objects.
///
/// The destructors of any contained objects will be called.
/// This function is invoked at exit, but if your application
/// needs to ensure objects are released before the other
/// destructors are called (since no ordering is guaranteed for
/// statically-allocated objects), you may call this at any time.
///
/// Libraries should not invoke this function; if your library
/// uses this singleton container, your code run at exit should
/// simply remove any stored objects rather than forcing all of
/// the application's singletons to be destroyed.
SMTKCORE_EXPORT void finalizeSingletons();

namespace detail
{

// Implementation detail for Schwarz counter idiom.
class SMTKCORE_EXPORT singletonsCleanup
{
public:
  singletonsCleanup();
  singletonsCleanup(const singletonsCleanup& other) = delete;
  singletonsCleanup& operator=(const singletonsCleanup& rhs) = delete;
  ~singletonsCleanup();
};

static singletonsCleanup singletonsCleanupInstance;

} // namespace detail

} // namespace common
} // namespace smtk

#endif // smtk_common_Singletons_h
