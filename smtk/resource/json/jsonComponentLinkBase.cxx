//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/json/jsonComponentLinkBase.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/ComponentLinks.h"

#include "smtk/common/json/jsonUUID.h"

namespace smtk
{
namespace resource
{
namespace detail
{
void from_json(const json&, smtk::resource::detail::ComponentLinkBase&)
{
}

void to_json(json&, const smtk::resource::detail::ComponentLinkBase&)
{
}
}
}
}
