//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/AuxiliaryGeometry.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/Manager.h"

namespace smtk
{
namespace model
{

/**\brief Indicate whether the entity has a URL to external geometry.
  *
  */
bool AuxiliaryGeometry::hasUrl() const
{
  if (!this->hasStringProperties())
  {
    return false;
  }
  const StringData& sprops(this->stringProperties());
  return (sprops.find("url") != sprops.end());
}

/**\brief Return the URL to external geometry if any exists, and an empty string otherwise.
  *
  * Note that only the first string (in an array of strings that may be held) is returned.
  */
std::string AuxiliaryGeometry::url() const
{
  if (!this->hasStringProperties())
  {
    return std::string();
  }
  const StringData& sprops(this->stringProperties());
  StringData::const_iterator url = sprops.find("url");
  if (url == sprops.end() || url->second.empty())
  {
    return std::string();
  }
  return url->second[0];
}

/**\brief Set the URL of external geometry referenced by this auxiliary geometry.
  *
  */
void AuxiliaryGeometry::setUrl(const std::string& url)
{
  this->setStringProperty("url", url);
}

} // namespace model
} // namespace smtk
