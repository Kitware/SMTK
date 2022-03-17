//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_VersionNumber_h
#define smtk_common_VersionNumber_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"

#include <array>
#include <string>

// On some systems, major and minor are defined as macros. If this is one of
// those systems, undefine these macros before defining the methods
// of smtk::common::VersionNumber.
#ifdef major
#undef major
#endif

#ifdef minor
#undef minor
#endif

namespace smtk
{
namespace common
{

/**\brief A 2- or 3-component version number (i.e., major, minor, and optional patch).
  *
  */
class SMTKCORE_EXPORT VersionNumber : public std::array<int, 3>
{
public:
  VersionNumber();
  VersionNumber(const VersionNumber& other);
  VersionNumber(int major, int minor = 0, int patch = 0);
  VersionNumber(const std::string& versionString);

  int major() const;
  int minor() const;
  int patch() const;

  std::string string() const;

  /// Returns true if the version number has been set; false otherwise.
  ///
  /// Note that VersionNumber instances are initialized with a negative
  /// major version number that marks them as invalid.
  bool isValid() const;

  VersionNumber& operator=(const VersionNumber&) = default;
};

SMTKCORE_EXPORT std::ostream& operator<<(std::ostream& stream, const VersionNumber& uid);
SMTKCORE_EXPORT std::istream& operator>>(std::istream& stream, VersionNumber& uid);

} // namespace common
} // namespace smtk

#endif // smtk_common_VersionNumber_h
