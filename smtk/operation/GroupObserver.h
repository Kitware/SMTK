//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_operation_GroupObserver_h
#define __smtk_operation_GroupObserver_h

#include "smtk/CoreExports.h"

#include "smtk/common/Observers.h"

#include "smtk/operation/Operation.h"

namespace smtk
{
namespace operation
{
class Group;

typedef std::function<void(const Operation::Index&, const std::string&, bool)> GroupObserver;

typedef smtk::common::Observers<GroupObserver> GroupObservers;
}
}

#endif // __smtk_operation_GroupObserver_h
