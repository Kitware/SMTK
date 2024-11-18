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
#include "PybindAgent.h"
#include "PybindFillOutAttributes.h"
#include "PybindFillOutAttributesAgent.h"
#include "PybindGatherObjectsAgent.h"
#include "PybindGatherResources.h"
#include "PybindGallery.h"
#include "PybindManager.h"
#include "PybindObjectsInRoles.h"
#include "PybindPort.h"
#include "PybindPortData.h"
#include "PybindRegistrar.h"
#include "PybindState.h"
#include "PybindSubmitOperationAgent.h"
#include "PybindTask.h"
#include "PybindWorklet.h"
#include "PybindInstances.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindTask, m)
{
  m.doc() = "SMTK tasks are portions of a workflow.";
  py::module smtk = m.def_submodule("smtk", "<description>");
  py::module task = smtk.def_submodule("task", "<description>");
  py::module::import("smtk.string");
  py::module::import("smtk.resource");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  auto smtk_task_Active = pybind11_init_smtk_task_Active(m);
  auto smtk_task_Manager = pybind11_init_smtk_task_Manager(m);
  auto smtk_task_Instances = pybind11_init_smtk_task_Instances(m);
  auto smtk_task_Registrar = pybind11_init_smtk_task_Registrar(m);
  auto smtk_task_Agent = pybind11_init_smtk_task_Agent(m);
  auto smtk_task_FillOutAttributesAgent = pybind11_init_smtk_task_FillOutAttributesAgent(m);
  auto smtk_task_SubmitOperationAgent = pybind11_init_smtk_task_SubmitOperationAgent(m);
  auto smtk_task_GatherObjectsAgent = pybind11_init_smtk_task_GatherObjectsAgent(m);
  auto smtk_task_Task = pybind11_init_smtk_task_Task(m);
  pybind11_init_smtk_task_State(m);
  pybind11_init_smtk_task_stateEnum(m);
  pybind11_init_smtk_task_stateName(m);
  auto smtk_task_PortData = pybind11_init_smtk_task_PortData(m);
  auto smtk_task_ObjectsInRoles = pybind11_init_smtk_task_ObjectsInRoles(m);
  auto smtk_task_Port = pybind11_init_smtk_task_Port(m);
  auto smtk_task_FillOutAttributes = pybind11_init_smtk_task_FillOutAttributes(m);
  auto smtk_task_GatherResources = pybind11_init_smtk_task_GatherResources(m);
  auto smtk_task_Worklet = pybind11_init_smtk_task_Worklet(m);
  auto smtk_task_Gallery = pybind11_init_smtk_task_Gallery(m);
}
