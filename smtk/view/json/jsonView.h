//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_jsonView_h
#define smtk_view_jsonView_h

#include "smtk/CoreExports.h"
#include "smtk/view/View.h"

#include "json.hpp"

#include <sstream>

namespace smtk
{
namespace view
{

SMTKCORE_EXPORT void to_json(nlohmann::json& j, const smtk::view::View::Component& comp);

SMTKCORE_EXPORT void from_json(const nlohmann::json& j, View::Component& comp);

SMTKCORE_EXPORT void to_json(nlohmann::json& j, const ViewPtr& view);

SMTKCORE_EXPORT void from_json(const nlohmann::json& j, smtk::view::ViewPtr& view);
}
}

#endif
