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

namespace smtk
{
namespace common
{

/**\brief Utilities for dealing with color conversion.
  */
class SMTKCORE_EXPORT Color
{
public:
  static bool
  stringToFloatRGBA(double* rgba, const std::string& colorSpec, double defaultAlpha = 1.0);
  /// Convenience to convert a string color specifier into a vector instead of a pointer.
  static bool stringToFloatRGBA(
    std::vector<double>& rgba,
    const std::string& colorSpec,
    double defaultAlpha = 1.0)
  {
    rgba.resize(4);
    return Color::stringToFloatRGBA(rgba.data(), colorSpec, defaultAlpha);
  }
  static std::string floatRGBAToString(const double* rgb);
  static std::string floatRGBAToString(const float* rgb);
  static std::string floatRGBToString(const double* rgb);
  static std::string floatRGBToString(const float* rgb);
  static double floatRGBToLightness(const double* rgb);
  static float floatRGBToLightness(const float* rgb);
};

} // namespace common
} // namespace smtk

#endif // smtk_common_Color_h
