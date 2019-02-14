//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_operation_Observer_h
#define __smtk_operation_Observer_h

#include "smtk/common/Observers.h"
#include "smtk/operation/Operation.h"

namespace smtk
{
namespace operation
{

/**\brief Enumerate events that an operation may encounter.
 *
 * No event is provided for operation deletion because
 * (1) operation deletion is not managed and
 * (2) "this" is not complete in class destructors (subclass data is already
 * freed).
 * So, there is no easy way to observe when an operation is about to be
 * deleted but is still valid.
 */
enum class EventType
{
  CREATED,      //!< An instance of the Operation class has been created.
  WILL_OPERATE, //!< The operation will commence if no observers cancel it.
  DID_OPERATE   //!< The operation has completed or been canceled.
};

typedef std::function<int(std::shared_ptr<Operation>, EventType, Operation::Result)> Observer;

typedef smtk::common::Observers<Observer> Observers;
}
}

#endif // __smtk_operation_Observer_h
