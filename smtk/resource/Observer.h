//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_resource_Observer_h
#define __smtk_resource_Observer_h

#include "smtk/common/Observers.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
namespace resource
{

/**\brief Enumerate events that the resource manager may encounter.
  *
  * Note that the resource modification event only fires when
  * a resource's clean/dirty state changes, not for each modification:
  * once a resource is dirty, further modifications are not reported.
  * If you want to monitor each modification to a resource, you should
  * observe operations via an operation manager.
  *
  * Also, the MODIFIED signal is fired when a resource is marked clean
  * not just when it is marked dirty.
  */
enum class EventType
{
  ADDED,    //!< A new resource's contents now available in memory.
  MODIFIED, //!< an existing resource's clean/dirty state has changed.
  REMOVED   //!< An existing resource's contents are being removed from memory.
};

typedef std::function<void(std::shared_ptr<Resource>, EventType)> Observer;

typedef smtk::common::Observers<Observer> Observers;
}
}

#endif // __smtk_resource_Observer_h
