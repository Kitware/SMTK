//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/resource/query/Query.h"

#include <typeindex>

namespace smtk
{
namespace resource
{
namespace query
{

std::size_t Query::typeIndex()
{
  return std::type_index(typeid(Query)).hash_code();
}

int Query::numberOfGenerationsFromType(std::size_t index)
{
  return (Query::typeIndex() == index ? 0 : std::numeric_limits<int>::lowest());
}

} // namespace query
} // namespace resource
} // namespace smtk
