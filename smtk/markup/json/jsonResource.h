//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_markup_json_jsonResource_h
#define smtk_markup_json_jsonResource_h

#include "smtk/markup/Exports.h"

#include "smtk/markup/IdNature.h"
#include "smtk/markup/Resource.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace markup
{

SMTKMARKUP_EXPORT void to_json(nlohmann::json& jj, const smtk::markup::Resource::Ptr& resource);
SMTKMARKUP_EXPORT void from_json(const nlohmann::json& jj, smtk::markup::Resource::Ptr& resource);

SMTKMARKUP_EXPORT void to_json(nlohmann::json& jj, const smtk::markup::IdNature& nature);
SMTKMARKUP_EXPORT void from_json(const nlohmann::json& jj, smtk::markup::IdNature& nature);

} // namespace markup
} // namespace smtk

#endif // smtk_markup_json_jsonResource_h
