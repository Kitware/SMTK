//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_PortForwardingAgent_h
#define pybind_smtk_task_PortForwardingAgent_h

#include <pybind11/pybind11.h>

#include "smtk/task/PortForwardingAgent.h"

#include "smtk/task/Port.h"

namespace py = pybind11;

inline py::class_< smtk::task::PortForwardingAgent > pybind11_init_smtk_task_PortForwardingAgent(py::module &m)
{
  py::class_< smtk::task::PortForwardingAgent, smtk::task::Agent > instance(m, "PortForwardingAgent");
  instance
    .def("typeToken", &smtk::task::PortForwardingAgent::typeToken)
    .def("classHierarchy", &smtk::task::PortForwardingAgent::classHierarchy)
    .def("matchesType", &smtk::task::PortForwardingAgent::matchesType, py::arg("candidate"))
    .def("generationsFromBase", &smtk::task::PortForwardingAgent::generationsFromBase, py::arg("base"))
    .def("state", &smtk::task::PortForwardingAgent::state)
    .def("configure", [](smtk::task::PortForwardingAgent& self, const std::string& jsonConfig)
      {
        auto config = nlohmann::json::parse(jsonConfig);
        self.configure(config);
      })
    .def("configuration", &smtk::task::PortForwardingAgent::configuration)
    .def("name", &smtk::task::PortForwardingAgent::name)
    .def("portData", [](smtk::task::PortForwardingAgent& self, smtk::task::Port::Ptr port)
      {
        return self.portData(port.get());
      }, py::arg("port"))
    .def("parent", &smtk::task::PortForwardingAgent::parent, py::return_value_policy::reference_internal)
    ;
  return instance;
}

#endif
