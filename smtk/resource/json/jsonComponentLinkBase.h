//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_json_jsonComponentLinkBase_h
#define smtk_resource_json_jsonComponentLinkBase_h

#include "smtk/CoreExports.h"

#include "smtk/resource/ComponentLinks.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace resource
{
namespace detail
{
using json = nlohmann::json;

SMTKCORE_EXPORT void from_json(const json&, smtk::resource::detail::ComponentLinkBase&);

SMTKCORE_EXPORT void to_json(json&, const smtk::resource::detail::ComponentLinkBase&);
} // namespace detail
} // namespace resource
} // namespace smtk

#endif
