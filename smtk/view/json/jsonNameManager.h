//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_json_jsonNameManager_h
#define smtk_view_json_jsonNameManager_h

#include "smtk/CoreExports.h"

#include "smtk/view/NameManager.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace view
{

SMTKCORE_EXPORT void to_json(nlohmann::json& jj, const smtk::view::NameManager::Ptr& nameManager);
SMTKCORE_EXPORT void from_json(const nlohmann::json& jj, smtk::view::NameManager::Ptr& nameManager);

} // namespace view
} // namespace smtk

#endif // smtk_view_json_jsonNameManager_h
