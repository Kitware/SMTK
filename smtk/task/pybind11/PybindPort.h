//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_Port_h
#define pybind_smtk_task_Port_h

#include <pybind11/pybind11.h>

#include "smtk/task/Port.h"

#include "smtk/task/Task.h"
#include "smtk/task/PortData.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::task::Port, smtk::resource::Component > pybind11_init_smtk_task_Port(py::module &m)
{
  PySharedPtrClass< smtk::task::Port, smtk::resource::Component > instance(m, "Port");
  instance
    .def("typeName", &smtk::task::Port::typeName)
    .def_static("create", (std::shared_ptr<smtk::task::Port> (*)()) &smtk::task::Port::create)
    .def("configure", [](smtk::task::Port& task, const std::string& jsonConfig)
      {
        auto config = nlohmann::json::parse(jsonConfig);
        task.configure(config);
      })
    .def("setName", &smtk::task::Port::setName, py::arg("name"))
    .def("direction", &smtk::task::Port::direction)
    .def("access", &smtk::task::Port::access)
    .def("dataTypes", (std::unordered_set<smtk::string::Token>& (smtk::task::Port::*)()) &smtk::task::Port::dataTypes,
      py::return_value_policy::reference_internal)
    .def("connections", [](smtk::task::Port& self)
      {
        // Convert raw to shared pointers.
        std::vector<std::shared_ptr<smtk::resource::PersistentObject>> result;
        result.reserve(self.connections().size());
        for (const auto& conn : self.connections())
        {
          result.push_back(conn->shared_from_this());
        }
        return result;
      })
    .def("portData", [](smtk::task::Port& self, const smtk::resource::PersistentObject::Ptr& obj)
      {
        return self.portData(obj.get());
      }, py::arg("object"))
    .def("style", &smtk::task::Port::style)
    .def("addStyle", &smtk::task::Port::addStyle, py::arg("styleClass"))
    .def("removeStyle", &smtk::task::Port::removeStyle, py::arg("styleClass"))
    .def("clearStyle", &smtk::task::Port::clearStyle)
    .def("parent", &smtk::task::Port::parent)
    .def("setParent", [](smtk::task::Port& self, const smtk::task::Task::Ptr& parent)
      {
        return self.setParent(parent.get());
      }, py::arg("parent"))
    ;
  py::enum_<smtk::task::Port::Direction>(instance, "Direction")
    .value("In", smtk::task::Port::Direction::In)
    .value("Out", smtk::task::Port::Direction::Out)
    .export_values();
  py::enum_<smtk::task::Port::Access>(instance, "Access")
    .value("Internal", smtk::task::Port::Access::Internal)
    .value("External", smtk::task::Port::Access::External)
    .export_values();
  return instance;
}

#endif
