//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_string_json_jsonManager_h
#define smtk_string_json_jsonManager_h

#include "smtk/CoreExports.h"

#include "smtk/string/Manager.h"

#include "nlohmann/json.hpp"

// Define how managed string tokens are serialized.
namespace smtk
{
namespace string
{
using json = nlohmann::json;

SMTKCORE_EXPORT void to_json(json&, const std::shared_ptr<Manager>&);

SMTKCORE_EXPORT void from_json(const json&, std::shared_ptr<Manager>&);

} // namespace string
} // namespace smtk

#endif
