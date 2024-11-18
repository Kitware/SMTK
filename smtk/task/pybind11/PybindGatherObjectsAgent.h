
//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_GatherObjectsAgent_h
#define pybind_smtk_task_GatherObjectsAgent_h

#include <pybind11/pybind11.h>

#include "smtk/task/GatherObjectsAgent.h"

#include "smtk/task/State.h"

namespace py = pybind11;

inline py::class_< smtk::task::GatherObjectsAgent, smtk::task::Agent > pybind11_init_smtk_task_GatherObjectsAgent(py::module &m)
{
  py::class_< smtk::task::GatherObjectsAgent, smtk::task::Agent > instance(m, "GatherObjectsAgent");
  instance
    .def("typeName", &smtk::task::GatherObjectsAgent::typeName)
    .def("addObjectInRole", [](
        smtk::task::GatherObjectsAgent& self,
        const smtk::resource::PersistentObject::Ptr& object,
        smtk::string::Token role,
        bool signal)
      {
        return self.addObjectInRole(object.get(), role, signal);
      }, py::arg("object"), py::arg("role"), py::arg("signal") = false)
    .def("removeObjectFromRole", [](
        smtk::task::GatherObjectsAgent& self,
        const smtk::resource::PersistentObject::Ptr& object,
        smtk::string::Token role,
        bool signal)
      {
        return self.removeObjectFromRole(object.get(), role, signal);
      }, py::arg("object"), py::arg("role"), py::arg("signal") = false)
    .def("clearOutputPort", &smtk::task::GatherObjectsAgent::clearOutputPort, py::arg("signal") = false)
    .def("outputPort", [](smtk::task::GatherObjectsAgent& self)
      {
        return self.outputPort()->shared_from_this();
      })
    .def("data", [](smtk::task::GatherObjectsAgent& self)
      {
        return self.portData(self.outputPort());
      }, py::return_value_policy::reference_internal)
    ;
  return instance;
}

#endif
