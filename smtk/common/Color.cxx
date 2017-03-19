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

#include <cctype>
#include <cmath>

namespace smtk {
  namespace common {

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
bool Color::stringToFloatRGBA(
  double* rgba, const std::string& colorSpec)
{
  if (colorSpec.empty()) return false;

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
          if (!ok) { break; }
          colorNumer += factor * digitValue;
          }
        tmp[cc] = colorNumer / colorDenom;
        }
      if (ok)
        {
        // Copy what we successfully converted.
        for (int cp = 0; cp < cc; ++cp) { rgba[cp] = tmp[cp]; }
        // Fill out any remaining components.
        for (; cc < 4; ++cc) { rgba[cc] = (cc == 3 ? 1.0 : 0.0); }
        return true;
        }
      }
    }

  return ok;
}

  } // namespace common
} // namespace smtk
