//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/groups/InternalGroup.h"

namespace smtk
{
namespace operation
{

bool InternalGroup::registerOperation(const std::string& typeName)
{
  return Group::registerOperation(typeName);
}

bool InternalGroup::registerOperation(const Operation::Index& index)
{
  return Group::registerOperation(index);
}
}
}
