//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_json_jsonResourceLinkBase_h
#define smtk_resource_json_jsonResourceLinkBase_h

#include "smtk/resource/ResourceLinks.h"

#include "nlohmann/json.hpp"

namespace nlohmann
{
template <>
struct adl_serializer<smtk::resource::detail::ResourceLinkBase>
{
  static smtk::resource::detail::ResourceLinkBase from_json(const json&);
  static void to_json(json&, const smtk::resource::detail::ResourceLinkBase&);
};
}

#endif
