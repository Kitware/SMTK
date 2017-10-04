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

#include "smtk/common/UUIDGenerator.h"

namespace smtk
{
namespace resource
{

Component::Component(const common::UUID& myID)
  : m_id(myID)
{
}

Component::Component()
{
  this->m_id = smtk::common::UUIDGenerator::instance().random();
}

Component::~Component()
{
}

} // namespace resource
} // namespace smtk
