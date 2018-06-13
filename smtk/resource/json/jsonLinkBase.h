//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_json_jsonLinkBase_h
#define smtk_resource_json_jsonLinkBase_h

#include "smtk/resource/ResourceLinks.h"

#include "nlohmann/json.hpp"

namespace nlohmann
{
template <>
struct adl_serializer<smtk::resource::detail::LinkBase>
{
  static smtk::resource::detail::LinkBase from_json(const json&);
  static void to_json(json&, const smtk::resource::detail::LinkBase&);
};
}

#endif
