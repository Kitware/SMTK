//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_Color_h
#define smtk_common_Color_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"

#include <string>
#include <vector>

namespace smtk {
  namespace common {

/**\brief Utilities for dealing with color conversion.
  */
class SMTKCORE_EXPORT Color
{
public:
  static bool stringToFloatRGBA(
    double* rgba, const std::string& colorSpec);
  /// Convenience to convert a string color specifier into a vector instead of a pointer.
  static bool stringToFloatRGBA(
    std::vector<double>& rgba, const std::string& colorSpec)
    {
    rgba.resize(4);
    return Color::stringToFloatRGBA(&rgba[0], colorSpec);
    }
};

  } // namespace common
} // namespace smtk

#endif // smtk_common_Color_h
