//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/Color.h"

#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>

namespace smtk
{
namespace common
{

static bool hexDigitValue(int& value, char digit)
{
  if (digit >= '0' && digit <= '9')
  {
    value = (digit - '0');
    return true;
  }
  digit = std::tolower(digit);
  if (digit >= 'a' && digit <= 'f')
  {
    value = (digit - 'a' + 10);
    return true;
  }
  return false;
}

/**\brief Convert a string color specifier (\a colorSpec) into RGBA values in [0,1]^4.
  *
  * This returns true when \a colorSpec is a valid color and false otherwise.
  * If false is returned, then \a rgba is unmodified.
  */
bool Color::stringToFloatRGBA(double* rgba, const std::string& colorSpec, double defaultAlpha)
{
  if (colorSpec.empty())
    return false;

  bool ok = false;
  if (colorSpec[0] == '#')
  { // Color specified as hexadecimal RGB(A) string.
    int numDigits = static_cast<int>(colorSpec.size()) - 1;
    int numComponents = numDigits % 3 == 0 ? 3 : (numDigits % 4 == 0 ? 4 : -1);
    if (numComponents > 0)
    {
      ok = true; // We have what seems to be a valid hex string. Try converting it.
      double tmp[4];
      int colorDepth = numDigits / numComponents;
      double colorDenom = std::pow(16.0, colorDepth) - 1.0;
      int cc;
      for (cc = 0; ok && (cc < numComponents); ++cc)
      {
        double colorNumer = 0;
        int factor = 1;
        for (int dd = 0; ok && (dd < colorDepth); ++dd, factor *= 16)
        {
          int digitValue;
          ok = hexDigitValue(digitValue, colorSpec[(cc + 1) * colorDepth - dd]);
          if (!ok)
          {
            break;
          }
          colorNumer += factor * digitValue;
        }
        tmp[cc] = colorNumer / colorDenom;
      }
      if (ok)
      {
        // Copy what we successfully converted.
        for (int cp = 0; cp < cc; ++cp)
        {
          rgba[cp] = tmp[cp];
        }
        // Fill out any remaining components.
        for (; cc < 4; ++cc)
        {
          rgba[cc] = (cc == 3 ? defaultAlpha : 0.0);
        }
        return true;
      }
    }
  }

  return ok;
}

std::string Color::floatRGBAToString(const double* rgba)
{
  std::array<int, 4> rgba_;
  for (int i = 0; i < 4; i++)
  {
    rgba_[i] = int(255 * rgba[i]);
  }

  char hexcol[10];
  snprintf(hexcol, sizeof hexcol, "#%02x%02x%02x%02x", rgba_[0], rgba_[1], rgba_[2], rgba_[3]);
  return std::string(hexcol);
}

std::string Color::floatRGBAToString(const float* rgba)
{
  std::array<int, 4> rgba_;
  for (int i = 0; i < 4; i++)
  {
    rgba_[i] = int(255 * rgba[i]);
  }

  char hexcol[10];
  snprintf(hexcol, sizeof hexcol, "#%02x%02x%02x%02x", rgba_[0], rgba_[1], rgba_[2], rgba_[3]);
  return std::string(hexcol);
}

std::string Color::floatRGBToString(const double* rgb)
{
  int rgb_[3];
  for (int i = 0; i < 3; i++)
  {
    rgb_[i] = int(255 * rgb[i]);
  }

  char hexcol[8];
  snprintf(hexcol, sizeof hexcol, "#%02x%02x%02x", rgb_[0], rgb_[1], rgb_[2]);
  return std::string(hexcol);
}

std::string Color::floatRGBToString(const float* rgb)
{
  int rgb_[3];
  for (int i = 0; i < 3; i++)
  {
    rgb_[i] = int(255 * rgb[i]);
  }

  char hexcol[8];
  snprintf(hexcol, sizeof hexcol, "#%02x%02x%02x", rgb_[0], rgb_[1], rgb_[2]);
  return std::string(hexcol);
}

double Color::floatRGBToLightness(const double* rgb)
{
  // Alternatively, this is faster but less accurate: (0.299*R + 0.587*G + 0.114*B)
  double r2 = rgb[0] * rgb[0];
  double g2 = rgb[1] * rgb[1];
  double b2 = rgb[2] * rgb[2];
  double lightness = std::sqrt(0.299 * r2 + 0.587 * g2 + 0.114 * b2);
  return lightness;
}

float Color::floatRGBToLightness(const float* rgb)
{
  // Alternatively, this is faster but less accurate: (0.299*R + 0.587*G + 0.114*B)
  float r2 = rgb[0] * rgb[0];
  float g2 = rgb[1] * rgb[1];
  float b2 = rgb[2] * rgb[2];
  float lightness = std::sqrt(0.299f * r2 + 0.587f * g2 + 0.114f * b2);
  return lightness;
}
} // namespace common
} // namespace smtk
