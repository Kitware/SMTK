//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Component.cxx - Abstract base class for CMB Resource Components
// .SECTION Description
// .SECTION See Also

#include "smtk/resource/Component.h"

#include "smtk/resource/Resource.h"

#include <cassert>

namespace smtk
{
namespace resource
{

Component::Component()
  : m_links(this)
  , m_properties(this)
{
}

Component::~Component() = default;

} // namespace resource
} // namespace smtk
