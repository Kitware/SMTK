//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_query_Cache_h
#define smtk_resource_query_Cache_h

#include "smtk/CoreExports.h"

#include "smtk/common/UUID.h"

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace smtk
{
namespace resource
{
namespace query
{
/// A base class for persistent data used in conjunction with Query types. Query
/// Caches are separated from Query insteances to facilitate shared access to
/// Cache data among multiple Queries.
struct SMTKCORE_EXPORT Cache
{
  virtual ~Cache() = default;
  virtual std::size_t index() const { return std::type_index(typeid(*this)).hash_code(); }
};
}
}
}

#endif
