//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_Event_h
#define smtk_resource_Event_h

namespace smtk
{
namespace resource
{

/**\brief Events that the resource manager can signal to observers.
  *
  * At this time, resource modification is not an event since modifications
  * should be made via operations, which can be observed via an operation manager.
  */
enum class Event
{
  RESOURCE_ADDED,  //!< A new resource's contents now available in memory.
  RESOURCE_REMOVED //!< An existing resource's contents are being removed from memory.
};
}
}

#endif
