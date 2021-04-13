//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_json_jsonSurrogate_h
#define smtk_resource_json_jsonSurrogate_h

#include "smtk/resource/Surrogate.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace resource
{
using json = nlohmann::json;

void to_json(json&, const Surrogate&);

Surrogate from_json(const json&);
} // namespace resource
} // namespace smtk

#endif
