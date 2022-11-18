//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_json_DomainDeserializer_h
#define smtk_markup_json_DomainDeserializer_h

#include "smtk/markup/Domain.h"

#include "smtk/io/Logger.h"

#include "smtk/common/TypeName.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace markup
{

template<typename Resource>
struct DomainDeserializer
{
  DomainDeserializer(Resource* resource, const json& source)
    : m_resource(resource)
    , m_source(source)
  {
    if (!m_resource)
    {
      throw std::invalid_argument("Deserializer requires a non-null resource");
    }
  }

  template<typename DomainType>
  void evaluate(std::size_t ii) const
  {
    (void)ii;
    if (!m_resource)
    {
      throw std::logic_error("No resource to place deserialized domains.");
    }
    std::string domainType = smtk::common::typeName<DomainType>();
    smtk::string::Token domainToken(domainType);
    auto it = m_source.find(domainType);
    if (it == m_source.end())
    {
      return;
    }
    const json& jDomainsOfType(*it);
    for (const auto& jDomain : jDomainsOfType)
    {
      auto domain = jDomain.get<std::shared_ptr<DomainType>>();
      if (domain)
      {
        m_resource->domains().insert(domainToken, domain);
      }
    }
  }

  Resource* m_resource = nullptr;
  const json& m_source;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_json_DomainDeserializer_h
