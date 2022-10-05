//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/VersionNumber.h"

#include "smtk/Regex.h"
#include <sstream>

namespace smtk
{
namespace common
{

/// Default constructor creates a nil VersionNumber (IsNull() == true).
VersionNumber::VersionNumber()
  : std::array<int, 3>{ { -1, 0, 0 } }
{
}

VersionNumber::VersionNumber(int major, int minor, int patch)
  : std::array<int, 3>{ { major, minor, patch } }
{
}

/// Copy constructor.
VersionNumber::VersionNumber(const VersionNumber& other) = default;

/// Construct a VersionNumber from a text string in either major.minor or major.minor.patch format.
VersionNumber::VersionNumber(const std::string& txt)
{
  const smtk::regex versionRegex("([0-9]+)\\.([0-9]+)\\.?([0-9]+)?");
  smtk::smatch versionMatch;
  if (!txt.empty())
  {
    if (smtk::regex_match(txt, versionMatch, versionRegex))
    {
      for (std::size_t ii = 1; ii < versionMatch.size() && ii < 4; ++ii)
      {
        auto num = versionMatch[ii].str();
        (*this)[ii - 1] = num.empty() ? 0 : std::stoul(num);
      }
    }
  }
  else
  {
    *this = VersionNumber{ -1, 0, 0 };
  }
}

int VersionNumber::major() const
{
  return (*this)[0];
}

int VersionNumber::minor() const
{
  return (*this)[1];
}

int VersionNumber::patch() const
{
  return (*this)[2];
}

std::string VersionNumber::string() const
{
  std::ostringstream result;
  result << (*this)[0] << "." << (*this)[1];
  if ((*this)[2] != 0)
  {
    result << "." << (*this)[2];
  }
  return result.str();
}

bool VersionNumber::isValid() const
{
  return (*this)[0] >= 0;
}

/// Write a VersionNumber to a stream (as a string).
std::ostream& operator<<(std::ostream& stream, const VersionNumber& version)
{
  stream << version.string();
  return stream;
}

/// Read a VersionNumber from a stream (as a string).
std::istream& operator>>(std::istream& stream, VersionNumber& uid)
{
  std::string txt;
  stream >> txt;
  uid = VersionNumber(txt);
  return stream;
}

} // namespace common
} // namespace smtk
