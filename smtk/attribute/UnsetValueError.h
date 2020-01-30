//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_UnsetValueError_h
#define smtk_attribute_UnsetValueError_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h" // quiet dll-interface base class warnings on windows

#include <stdexcept>

namespace smtk
{
namespace attribute
{

/**\brief An exception for dereferencing an iterator to an unset Item value.
  */
class SMTKCORE_EXPORT UnsetValueError : public std::runtime_error
{
public:
  UnsetValueError()
    : std::runtime_error("Iterator to unset item cannot be dereferenced")
  {
  }
};

} // namespace attribute
} // namespace smtk

#endif
