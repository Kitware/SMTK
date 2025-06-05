//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_TrivialProducerAgent_h
#define pybind_smtk_task_TrivialProducerAgent_h

#include <pybind11/pybind11.h>

#include "smtk/task/TrivialProducerAgent.h"

#include "smtk/task/Port.h"

namespace py = pybind11;

inline py::class_< smtk::task::TrivialProducerAgent > pybind11_init_smtk_task_TrivialProducerAgent(py::module &m)
{
  py::class_< smtk::task::TrivialProducerAgent, smtk::task::Agent > instance(m, "TrivialProducerAgent");
  instance
    .def_static("addObjectInRole", [](
        smtk::task::Task* task, const std::string& agentName, const std::string& role, const smtk::resource::PersistentObject::Ptr& object)
      {
        return smtk::task::TrivialProducerAgent::addObjectInRole(task, agentName, role, object.get());
      },
      py::arg("task"), py::arg("agentName"), py::arg("role"), py::arg("object"))
    .def_static("addObjectInRole", [](
        smtk::task::Task* task, smtk::task::Port* port, const std::string& role, const smtk::resource::PersistentObject::Ptr& object)
      {
        return smtk::task::TrivialProducerAgent::addObjectInRole(task, port, role, object.get());
      },
      py::arg("task"), py::arg("port"), py::arg("role"), py::arg("object"))
    .def_static("removeObjectFromRole", [](
        smtk::task::Task* task, const std::string& agentName, const std::string& role, const smtk::resource::PersistentObject::Ptr& object)
      {
        return smtk::task::TrivialProducerAgent::removeObjectFromRole(task, agentName, role, object.get());
      },
      py::arg("task"), py::arg("agentName"), py::arg("role"), py::arg("object"))
    .def_static("removeObjectFromRole", [](
        smtk::task::Task* task, smtk::task::Port* port, const std::string& role, const smtk::resource::PersistentObject::Ptr& object)
      {
        return smtk::task::TrivialProducerAgent::removeObjectFromRole(task, port, role, object.get());
      },
      py::arg("task"), py::arg("port"), py::arg("role"), py::arg("object"))
    .def_static("resetData", [](smtk::task::Task* task, const std::string& agentName)
      {
        return smtk::task::TrivialProducerAgent::resetData(task, agentName);
      }, py::arg("task"), py::arg("agentName"))
    .def_static("resetData", [](smtk::task::Task* task, smtk::task::Port* port)
      {
        return smtk::task::TrivialProducerAgent::resetData(task, port);
      }, py::arg("task"), py::arg("port"))
    .def("typeToken", &smtk::task::TrivialProducerAgent::typeToken)
    .def("classHierarchy", &smtk::task::TrivialProducerAgent::classHierarchy)
    .def("matchesType", &smtk::task::TrivialProducerAgent::matchesType, py::arg("candidate"))
    .def("generationsFromBase", &smtk::task::TrivialProducerAgent::generationsFromBase, py::arg("base"))
    .def("state", &smtk::task::TrivialProducerAgent::state)
    .def("configure", [](smtk::task::TrivialProducerAgent& self, const std::string& jsonConfig)
      {
        auto config = nlohmann::json::parse(jsonConfig);
        self.configure(config);
      })
    .def("configuration", &smtk::task::TrivialProducerAgent::configuration)
    .def("name", &smtk::task::TrivialProducerAgent::name)
    .def("portData", [](smtk::task::TrivialProducerAgent& self, smtk::task::Port::Ptr port)
      {
        return self.portData(port.get());
      }, py::arg("port"))
    .def("parent", &smtk::task::TrivialProducerAgent::parent, py::return_value_policy::reference_internal)
    ;
  return instance;
}

#endif
