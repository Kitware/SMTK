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

#include "smtk/operation/NewOp.h"

#include <map>

namespace smtk
{
namespace operation
{

/**\brief Enumerate events that an operator may encounter.
 *
 * No event is provided for operator deletion because
 * (1) operator deletion is not managed and
 * (2) "this" is not complete in class destructors (subclass data is already
 * freed).
 * So, there is no easy way to observe when an operator is about to be
 * deleted but is still valid.
 */
enum class EventType
{
  CREATED,      //!< An instance of the Operator class has been created.
  WILL_OPERATE, //!< The operation will commence if no observers cancel it.
  DID_OPERATE   //!< The operation has completed or been canceled.
};

typedef std::function<int(std::shared_ptr<NewOp>, EventType, NewOp::Result)> Observer;

class SMTKCORE_EXPORT Observers
{
public:
  typedef int Key;

  /// Iterate over the collection of observers and execute the observer functor.
  /// Returns the bitwise-or of all observer return values.
  int operator()(std::shared_ptr<NewOp>, EventType, NewOp::Result);

  /// Ask to receive notification (and possibly a chance to cancel) events on
  /// all operations. The return value is a handle that can be used to
  /// unregister the observer.
  Key insert(Observer);

  /// Indicate that an observer should no longer be called. Returns the number
  /// of remaining observers.
  std::size_t erase(Key);

private:
  // A map of observers. The observers are held in a map so that they can be
  // referenced (and therefore removed) at a later time using the observer's
  // associated key.
  std::map<Key, Observer> m_observers;
};
}
}

#endif // __smtk_operation_Observer_h
