//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/json/jsonLinks.h"

namespace smtk
{
namespace common
{
namespace detail
{
void to_json(json& /*unused*/, const NullLinkBase& /*unused*/) {}

void from_json(const json& /*unused*/, NullLinkBase& /*unused*/) {}
} // namespace detail
} // namespace common
} // namespace smtk
