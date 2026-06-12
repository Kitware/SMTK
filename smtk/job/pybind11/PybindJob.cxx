//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/functional.h>
#include <utility>
SMTK_THIRDPARTY_POST_INCLUDE

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "PybindDatabaseQueue.h"
#include "PybindDefinition.h"
#include "PybindDefinitionInstances.h"
#include "PybindQueueInstances.h"
#include "PybindJob.h"
#include "PybindState.h"
#include "PybindStatus.h"
#include "PybindStage.h"
#include "PybindQueue.h"
#include "PybindManager.h"
#include "PybindRegistrar.h"

#include "smtk/common/UUID.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindJob, job_module)
{
  job_module.doc() = "Long-running job support";

  py::module::import("smtk.string");
  py::module::import("smtk.common");
  py::module::import("smtk.resource");
  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.

  pybind11_init_smtk_job_State(job_module);
  pybind11_init_smtk_job_Status(job_module);
  auto smtk_job_Definition = pybind11_init_smtk_job_Definition(job_module);
  auto smtk_job_Stage = pybind11_init_smtk_job_Stage(job_module);
  auto smtk_job_Job = pybind11_init_smtk_job_Job(job_module);
  auto smtk_job_Queue = pybind11_init_smtk_job_Queue(job_module);
  auto smtk_job_QueueInstances = pybind11_init_smtk_job_QueueInstances(job_module);
  auto smtk_job_DefinitionInstances = pybind11_init_smtk_job_DefinitionInstances(job_module);
  auto smtk_job_Manager = pybind11_init_smtk_job_Manager(job_module);
  auto smtk_job_DatabaseQueue = pybind11_init_smtk_job_DatabaseQueue(job_module);
  auto smtk_job_Registrar = pybind11_init_smtk_job_Registrar(job_module);

  // job_module
  //   .def("foo", &smtk::job::foo);

  // py::class_< smtk::job::Registrar > smtk_job_Registrar = pybind11_init_smtk_job_Registrar(job_module);
}
