//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_Agent_h
#define pybind_smtk_task_Agent_h

#include <pybind11/pybind11.h>

#include "smtk/task/Agent.h"

#include "smtk/task/Port.h"

namespace py = pybind11;

inline py::class_< smtk::task::Agent > pybind11_init_smtk_task_Agent(py::module &m)
{
  py::class_< smtk::task::Agent > instance(m, "Agent");
  instance
    .def("typeName", &smtk::task::Agent::typeName)
    .def("typeToken", &smtk::task::Agent::typeToken)
    .def("classHierarchy", &smtk::task::Agent::classHierarchy)
    .def("matchesType", &smtk::task::Agent::matchesType, py::arg("candidate"))
    .def("generationsFromBase", &smtk::task::Agent::generationsFromBase, py::arg("base"))
    .def("state", &smtk::task::Agent::state)
    .def("configure", [](smtk::task::Agent& self, const std::string& jsonConfig)
      {
        auto config = nlohmann::json::parse(jsonConfig);
        self.configure(config);
      })
    .def("configuration", &smtk::task::Agent::configuration)
    .def("portData", [](smtk::task::Agent& self, smtk::task::Port::Ptr port)
      {
        return self.portData(port.get());
      }, py::arg("port"))
    .def("parent", &smtk::task::Agent::parent, py::return_value_policy::reference_internal)
    ;
  return instance;
}

#endif
