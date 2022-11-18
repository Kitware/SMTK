//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_json_DomainSerializer_h
#define smtk_markup_json_DomainSerializer_h

#include "smtk/markup/Domain.h"

#include "smtk/common/TypeName.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace markup
{

template<typename Resource>
struct DomainSerializer
{
  DomainSerializer(const Resource* resource, nlohmann::json& destination)
    : m_resource(resource)
    , m_destination(destination)
  {
  }

  template<typename DomainType>
  void evaluate(std::size_t ii) const
  {
    (void)ii;
    if (!m_resource)
    {
      return;
    }
    nlohmann::json jDomainsOfType;
    std::string domainType = smtk::common::typeName<DomainType>();
    for (const auto& domainKey : m_resource->domains().keys())
    {
      if (const auto& domain = m_resource->domains().template findAs<DomainType>(domainKey))
      {
        if (domain->typeName() != domainType)
        {
          // Ignore subclasses.
          continue;
        }
        nlohmann::json jDomain;
        to_json(jDomain, domain.get());
        jDomainsOfType.push_back(jDomain);
      }
    }
    if (!jDomainsOfType.empty())
    {
      m_destination[domainType] = jDomainsOfType;
    }
  }

  const Resource* m_resource = nullptr;
  nlohmann::json& m_destination;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_json_DomainSerializer_h
