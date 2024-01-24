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

} // namespace markup
} // namespace smtk
