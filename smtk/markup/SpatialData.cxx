//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/SpatialData.h"

#include "smtk/markup/Domain.h"

namespace smtk
{
namespace markup
{

SpatialData::~SpatialData() = default;

void SpatialData::initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper)
{
  (void)data;
  (void)helper;
}

std::unordered_set<Domain*> SpatialData::domains() const
{
  // Return an empty set; children must override.
  std::unordered_set<Domain*> result;
  return result;
}

AssignedIds* SpatialData::domainExtent(Domain* domain) const
{
  if (!domain)
  {
    return nullptr;
  }
  return this->domainExtent(domain->name());
}

AssignedIds* SpatialData::domainExtent(smtk::string::Token domainName) const
{
  // Return null; children must override.
  (void)domainName;
  return nullptr;
}

bool SpatialData::isBlanked() const
{
  const auto& boolProps = this->properties().get<bool>();
  if (!boolProps.contains("_blanked"))
  {
    return false;
  }
  return boolProps.at("_blanked");
}

bool SpatialData::setBlanking(bool shouldBlank)
{
  if (!this->properties().contains<bool>("_blanked"))
  {
    // Already not blanked? Do nothing.
    if (!shouldBlank)
    {
      return false;
    }
    this->properties().emplace<bool>("_blanked", true);
    return true;
  }
  // Already blanked? Do nothing
  if (this->properties().at<bool>("_blanked") == shouldBlank)
  {
    return false;
  }
  if (shouldBlank)
  {
    this->properties().emplace<bool>("_blanked", true);
    return true;
  }
  this->properties().erase<bool>("_blanked");
  return true;
}

} // namespace markup
} // namespace smtk
