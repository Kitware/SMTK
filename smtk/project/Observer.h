//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_project_Observer_h
#define smtk_project_Observer_h

#include "smtk/common/Observers.h"
#include "smtk/project/Project.h"

namespace smtk
{
namespace project
{

/// Enumerate events that the project manager may encounter.
enum class EventType
{
  ADDED,    //!< A new project's contents now available in memory.
  MODIFIED, //!< An existing project's contents have been modified.
  REMOVED   //!< An existing project's contents are being removed from memory.
};

typedef std::function<void(const Project&, EventType)> Observer;

typedef smtk::common::Observers<Observer> Observers;
} // namespace project
} // namespace smtk

#endif
