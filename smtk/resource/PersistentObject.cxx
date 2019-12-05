//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/PersistentObject.h"

namespace smtk
{
namespace resource
{

PersistentObject::PersistentObject() = default;

PersistentObject::~PersistentObject() = default;

std::string PersistentObject::name() const
{
  return this->id().toString();
}
} // namespace resource
} // namespace smtk
