//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_query_BadTypeError_h
#define smtk_resource_query_BadTypeError_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"

#include "smtk/common/TypeName.h"

#include <stdexcept>

namespace smtk
{
namespace resource
{
namespace query
{
/// An exception to throw when a type is requested that does not exist in a
/// container.
///
/// SMTK's query system uses types to index into maps and retrieve instances of
/// that type. Much like how a map throws an exception when a key is not present
/// in the map, this exception is thrown when a type is not present in a
/// container.
class SMTKCORE_EXPORT BadTypeError : public std::out_of_range
{
public:
  BadTypeError(const std::string& typeName)
    : std::out_of_range("Type \"" + typeName + "\" not available in this container")
  {
  }
};
}
}
}

#endif
