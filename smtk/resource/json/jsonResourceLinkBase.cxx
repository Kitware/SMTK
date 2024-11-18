//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/json/jsonResourceLinkBase.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/ComponentLinks.h"

#include "smtk/common/json/jsonLinks.h"
#include "smtk/common/json/jsonUUID.h"

#include "smtk/resource/json/jsonComponentLinkBase.h"
#include "smtk/resource/json/jsonSurrogate.h"

// Ignore warning about non-inlined template specializations of smtk::common::Helper<>
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
#pragma warning(disable : 4506) /* no definition for inline function */
#endif

namespace nlohmann
{
smtk::resource::detail::ResourceLinkBase
adl_serializer<smtk::resource::detail::ResourceLinkBase>::from_json(const json& j)
{
  smtk::resource::Surrogate surrogate = smtk::resource::from_json(j["surrogate"]);
  smtk::resource::detail::ResourceLinkBase linkBase(std::move(surrogate));
  smtk::common::from_json(
    j["linkData"], static_cast<smtk::resource::Component::Links::Data&>(linkBase));
  return linkBase;
}

void adl_serializer<smtk::resource::detail::ResourceLinkBase>::to_json(
  json& j,
  const smtk::resource::detail::ResourceLinkBase& linkBase)
{
  j["surrogate"] = static_cast<const smtk::resource::Surrogate&>(linkBase);
  j["linkData"] = static_cast<const smtk::resource::Component::Links::Data&>(linkBase);
}
} // namespace nlohmann
