//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/Artifact.h"

namespace smtk
{
namespace resource
{

bool Artifact::setLocation(URL location)
{
  if (m_location == location)
  {
    return false;
  }
  m_location = location;
  return true;
}

bool Artifact::setChecksum(smtk::string::Token algorithm, const std::vector<std::uint8_t>& value)
{
  if (m_checksumAlgorithm == algorithm && m_checksumData == value)
  {
    return false;
  }
  m_checksumAlgorithm = algorithm;
  m_checksumData = value;
  return true;
}

bool Artifact::setTimestamp(smtk::string::Token format, const std::vector<std::uint8_t>& value)
{
  if (m_timestampFormat == format && m_timestampData == value)
  {
    return false;
  }
  m_timestampFormat = format;
  m_timestampData = value;
  return true;
}

bool Artifact::setExtant(bool isExtant)
{
  if (m_extant == isExtant)
  {
    return false;
  }
  m_extant = isExtant;
  return true;
}

} // namespace resource
} // namespace smtk
