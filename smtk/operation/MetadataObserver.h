//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_operation_MetadataObserver_h
#define __smtk_operation_MetadataObserver_h

#include "smtk/CoreExports.h"

#include <functional>
#include <map>

namespace smtk
{
namespace operation
{
class Metadata;

typedef std::function<void(const Metadata&)> MetadataObserver;

class SMTKCORE_EXPORT MetadataObservers
{
public:
  typedef int Key;

  MetadataObservers();
  ~MetadataObservers();

  /// Iterate over the collection of observers and execute the observer functor.
  void operator()(const Metadata&);

  /// Ask to receive notification when operations are added. The return value is
  /// a handle that can be used to unregister the observer.
  Key insert(MetadataObserver);

  /// Indicate that an observer should no longer be called. Returns the number
  /// of remaining observers.
  std::size_t erase(Key);

private:
  // A map of observers. The observers are held in a map so that they can be
  // referenced (and therefore removed) at a later time using the observer's
  // associated key.
  struct Internal;
  Internal* m_internal;
};
}
}

#ifndef smtkCore_EXPORTS
extern
#endif
  template class SMTKCORE_EXPORT std::function<void(const smtk::operation::Metadata&)>;

#endif // __smtk_operation_MetadataObserver_h
