//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/task/json/Configurator.h"
#include "smtk/task/json/Configurator.txx"

#include "smtk/task/Adaptor.h"
#include "smtk/task/Task.h"

static std::mutex g_types;

namespace smtk
{
namespace task
{
namespace json
{

std::mutex& typeMutex()
{
  return g_types;
}

// template<> Configurator<Task, typeMutex>::HelperTypeMap Configurator<Task, &typeMutex>::s_types;
// template<> Configurator<Adaptor, typeMutex>::HelperTypeMap Configurator<Adaptor, &typeMutex>::s_types;

} // namespace json
} // namespace task
} // namespace smtk
