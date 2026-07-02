//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_job_Registrar_h
#define smtk_job_Registrar_h

#include "smtk/CoreExports.h"

#include "smtk/attribute/Registrar.h"
#include "smtk/common/Managers.h"
#include "smtk/job/Manager.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Registrar.h"
#include "smtk/task/Manager.h"

namespace smtk
{
namespace job
{
class SMTKCORE_EXPORT Registrar
{
public:
  using Dependencies = std::tuple<attribute::Registrar, resource::Registrar, operation::Registrar>;

  static void registerTo(const smtk::common::Managers::Ptr&);
  static void unregisterFrom(const smtk::common::Managers::Ptr&);

  static void registerTo(const smtk::resource::Manager::Ptr&);
  static void unregisterFrom(const smtk::resource::Manager::Ptr&);

  static void registerTo(const smtk::operation::Manager::Ptr&);
  static void unregisterFrom(const smtk::operation::Manager::Ptr&);

  static void registerTo(const smtk::task::Manager::Ptr&);
  static void unregisterFrom(const smtk::task::Manager::Ptr&);

  static void registerTo(const smtk::job::Manager::Ptr&);
  static void unregisterFrom(const smtk::job::Manager::Ptr&);
};
} // namespace job
} // namespace smtk

#endif
