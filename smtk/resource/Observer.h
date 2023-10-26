//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_Observer_h
#define smtk_resource_Observer_h

#include "smtk/common/Observers.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
namespace resource
{

/**\brief Enumerate events that the resource manager may encounter.
  */
enum class EventType
{
  ADDED,  //!< A new resource's contents now available in memory.
  REMOVED //!< An existing resource's contents are being removed from memory.
};

typedef std::function<void(const Resource&, EventType)> Observer;

typedef smtk::common::Observers<Observer> Observers;
} // namespace resource
} // namespace smtk

#endif // smtk_resource_Observer_h
