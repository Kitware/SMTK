//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_SynchronizedCache_h
#define smtk_operation_SynchronizedCache_h

#include "smtk/CoreExports.h"

#include "smtk/resource/query/Cache.h"

#include "smtk/operation/Operation.h"

namespace smtk
{
namespace operation
{

/**\brief A query cache that updates according to the result outputs of operations.
  *
  * By default, caches are populated when they are used the first time and then
  * their contents are used for subsequent calls; there is no default mechanism
  * to flush a cache because there is no appropriate signal to trigger the cache
  * to flush. A SynchronizedCache can be registered to an operation Manager to
  * flush its cache elements that correspond to modified or expunged components.
  */
struct SMTKCORE_EXPORT SynchronizedCache : public smtk::resource::query::Cache
{
  virtual void synchronize(const Operation&, const Operation::Result&) = 0;
};
}
}

#endif
