//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_paraview_appcomponents_pqSMTKBehavior_h
#define pybind_smtk_extension_paraview_appcomponents_pqSMTKBehavior_h

#include <pybind11/pybind11.h>

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"
#include "smtk/extension/vtk/pybind11/PybindVTKTypeCaster.h"

#include "smtk/common/Managers.h"
#include "smtk/operation/Manager.h"
#include "smtk/project/Manager.h"
#include "smtk/resource/Manager.h"
#include "smtk/task/Manager.h"
#include "smtk/view/AvailableOperations.h"
#include "smtk/view/Manager.h"
#include "smtk/view/Selection.h"

#include "pqServer.h"
#include "pqView.h"

namespace py = pybind11;

inline py::class_< pqSMTKBehavior, QObject > pybind11_init_pqSMTKBehavior(py::module &m)
{
  py::class_< pqSMTKBehavior, QObject > instance(m, "pqSMTKBehavior");
  instance
    .def("addPQProxy", &pqSMTKBehavior::addPQProxy, py::arg("rsrcMgr"))
    .def("addedManagerOnServer", (void (pqSMTKBehavior::*)(::vtkSMSMTKWrapperProxy *, ::pqServer *)) &pqSMTKBehavior::addedManagerOnServer, py::arg("mgr"), py::arg("server"))
    .def("addedManagerOnServer", (void (pqSMTKBehavior::*)(::pqSMTKWrapper *, ::pqServer *)) &pqSMTKBehavior::addedManagerOnServer, py::arg("mgr"), py::arg("server"))
    .def("builtinOrActiveWrapper", &pqSMTKBehavior::builtinOrActiveWrapper)
    .def("createRepresentation", &pqSMTKBehavior::createRepresentation, py::arg("pipelinesource"), py::arg("view"))
    .def("getPVResource", &pqSMTKBehavior::getPVResource, py::arg("resource"))
    .def("getPVResourceProxy", &pqSMTKBehavior::getPVResourceProxy, py::arg("resource"))
    .def("getPVResourceManager", &pqSMTKBehavior::getPVResourceManager, py::arg("resourceManager"))
    .def_static("instance", &pqSMTKBehavior::instance, py::arg("parent") = nullptr)
    .def_static("processEvents", &pqSMTKBehavior::processEvents)
    .def("removingManagerFromServer", (void (pqSMTKBehavior::*)(::vtkSMSMTKWrapperProxy *, ::pqServer *)) &pqSMTKBehavior::removingManagerFromServer, py::arg("mgr"), py::arg("server"))
    .def("removingManagerFromServer", (void (pqSMTKBehavior::*)(::pqSMTKWrapper *, ::pqServer *)) &pqSMTKBehavior::removingManagerFromServer, py::arg("mgr"), py::arg("server"))
    .def("resourceManagerForServer", &pqSMTKBehavior::resourceManagerForServer, py::arg("server") = nullptr)
    .def("visitResourceManagersOnServers", &pqSMTKBehavior::visitResourceManagersOnServers, py::arg("fn"))
    .def("wrapperProxy", [](pqSMTKBehavior& behavior)
      { return behavior.wrapperProxy(); })// &pqSMTKBehavior::wrapperProxy, py::arg("server") = nullptr)
    .def("activeWrapperResourceManager", [](pqSMTKBehavior& behavior)
      { return behavior.wrapperProxy()->GetResourceManager(); })
    .def("activeWrapperOperationManager", [](pqSMTKBehavior& behavior)
      { return behavior.wrapperProxy()->GetOperationManager(); })
    .def("activeWrapperViewManager", [](pqSMTKBehavior& behavior)
      { return behavior.wrapperProxy()->GetViewManager(); })
    .def("activeWrapperSelection", [](pqSMTKBehavior& behavior)
      { return behavior.wrapperProxy()->GetSelection(); })
    .def("activeWrapperTaskManager", [](pqSMTKBehavior& behavior)
      { return behavior.wrapperProxy()->GetManagersPtr()->get<smtk::task::Manager::Ptr>(); })
    .def("activeWrapperProjectManager", [](pqSMTKBehavior& behavior)
      { return behavior.wrapperProxy()->GetProjectManager(); })
    ;
  return instance;
}

#endif
