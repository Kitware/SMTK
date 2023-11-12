//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_Registrar_h
#define smtk_task_Registrar_h

#include "smtk/CoreExports.h"

#include "smtk/common/Managers.h"
#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Registrar.h"
#include "smtk/task/Manager.h"

namespace smtk
{
namespace task
{
class SMTKCORE_EXPORT Registrar
{
public:
  using Dependencies = std::tuple<resource::Registrar>;

  static void registerTo(const smtk::task::Manager::Ptr&);
  static void unregisterFrom(const smtk::task::Manager::Ptr&);

  static void registerTo(const smtk::operation::Manager::Ptr&);
  static void unregisterFrom(const smtk::operation::Manager::Ptr&);
};
} // namespace task
} // namespace smtk

#endif // smtk_task_Registrar_h
