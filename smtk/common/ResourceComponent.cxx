//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkResourceComponent.cxx - Abstract base class for CMB ResourceComponents
// .SECTION Description
// .SECTION See Also

#include "smtk/common/ResourceComponent.h"
#include "smtk/common/UUIDGenerator.h"

namespace smtk
{
namespace common
{

ResourceComponent::ResourceComponent(const UUID& myID)
  : m_id(myID)
{
}

ResourceComponent::ResourceComponent()
{
  smtk::common::UUIDGenerator gen;
  this->m_id = gen.random();
}

ResourceComponent::~ResourceComponent()
{
}

} // namespace common
} // namespace smtk
