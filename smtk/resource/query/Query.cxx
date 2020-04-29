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
const std::size_t Query::type_index = std::type_index(typeid(Query)).hash_code();
}
}
}
