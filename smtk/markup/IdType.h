//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_IdType_h
#define smtk_markup_IdType_h

#include "smtk/markup/Exports.h"

#include <cstddef> // for std::size_t

namespace smtk
{
namespace markup
{

/// The type used for holding IDs in an IdSpace domain.
using IdType = std::size_t;

/// A value used to indicate no ID is available or a reference is invalid.
constexpr IdType SMTKMARKUP_EXPORT InvalidId()
{
  return ~0ull;
}

} // namespace markup
} // namespace smtk

#endif // smtk_markup_IdType_h
