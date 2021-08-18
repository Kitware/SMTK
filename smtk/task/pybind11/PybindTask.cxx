//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <pybind11/pybind11.h>
#include <utility>

#include "nlohmann/json.hpp"

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

using namespace nlohmann;

#include "PybindActive.h"
#include "PybindFillOutAttributes.h"
#include "PybindGatherResources.h"
#include "PybindManager.h"
#include "PybindRegistrar.h"
#include "PybindState.h"
#include "PybindTask.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(task, m)
{
  m.doc() = "SMTK tasks are portions of a workflow.";
  py::module smtk = m.def_submodule("smtk", "<description>");
  py::module task = smtk.def_submodule("task", "<description>");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  auto smtk_task_Active = pybind11_init_smtk_task_Active(task);
  auto smtk_task_Manager = pybind11_init_smtk_task_Manager(task);
  auto smtk_task_Registrar = pybind11_init_smtk_task_Registrar(task);
  auto smtk_task_Task = pybind11_init_smtk_task_Task(task);
  pybind11_init_smtk_task_State(task);
  pybind11_init_smtk_task_stateEnum(task);
  pybind11_init_smtk_task_stateName(task);
  auto smtk_task_FillOutAttributes = pybind11_init_smtk_task_FillOutAttributes(task);
  auto smtk_task_GatherResources = pybind11_init_smtk_task_GatherResources(task);
}
