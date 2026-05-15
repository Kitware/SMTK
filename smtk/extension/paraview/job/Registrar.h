//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_job_Registrar_h
#define smtk_extension_paraview_job_Registrar_h
#ifndef __VTK_WRAP__

#include "smtk/extension/paraview/job/smtkPVJobExtModule.h"

#include "smtk/job/Manager.h"
#include "smtk/job/Registrar.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/resource/Manager.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace job
{

class SMTKPVJOBEXT_EXPORT Registrar
{
public:
  using Dependencies = std::tuple<operation::Registrar, job::Registrar>;

  static void registerTo(const smtk::common::Managers::Ptr&);
  static void unregisterFrom(const smtk::common::Managers::Ptr&);

  static void registerTo(const smtk::job::Manager::Ptr&);
  static void unregisterFrom(const smtk::job::Manager::Ptr&);
};
} // namespace job
} // namespace paraview
} // namespace extension
} // namespace smtk

#endif // __VTK_WRAP__
#endif // smtk_extension_paraview_job_Registrar_h
