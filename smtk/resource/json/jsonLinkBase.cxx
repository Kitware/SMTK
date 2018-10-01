//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/json/jsonLinkBase.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/ComponentLinks.h"

#include "smtk/common/json/jsonLinks.h"
#include "smtk/common/json/jsonUUID.h"
#include "smtk/resource/json/jsonSurrogate.h"

namespace nlohmann
{
smtk::resource::detail::LinkBase adl_serializer<smtk::resource::detail::LinkBase>::from_json(
  const json& j)
{
  smtk::resource::Surrogate surrogate = smtk::resource::from_json(j);
  smtk::resource::detail::LinkBase linkBase(std::move(surrogate));
  smtk::common::from_json(j, static_cast<smtk::resource::Component::Links::Data&>(linkBase));
  return linkBase;
}

void adl_serializer<smtk::resource::detail::LinkBase>::to_json(
  json& j, const smtk::resource::detail::LinkBase& linkBase)
{
  smtk::resource::to_json(j, static_cast<const smtk::resource::Surrogate&>(linkBase));
  smtk::common::to_json(j, static_cast<const smtk::resource::Component::Links::Data&>(linkBase));
}
}
