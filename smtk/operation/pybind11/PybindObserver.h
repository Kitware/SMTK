//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_Observer_h
#define pybind_smtk_operation_Observer_h

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

#include "smtk/attribute/Attribute.h"

#include "smtk/operation/Observer.h"

namespace py = pybind11;

inline void pybind11_init_smtk_operation_EventType(py::module &m)
{
  py::enum_<smtk::operation::EventType>(m, "EventType")
    .value("WILL_OPERATE", smtk::operation::EventType::WILL_OPERATE)
    .value("DID_OPERATE", smtk::operation::EventType::DID_OPERATE)
    .export_values();
}

inline py::class_< smtk::operation::Observers > pybind11_init_smtk_operation_Observers(py::module &m)
{
  py::class_< smtk::operation::Observers > instance(m, "Observers");
  instance
    .def(py::init<>())
    .def("__call__", [](smtk::operation::Observers& observers, smtk::operation::Operation& op, ::smtk::operation::EventType eventType, ::smtk::operation::Operation::Result result) { return observers(op, eventType, result); })
    .def("__len__", &smtk::operation::Observers::size)
    .def("erase", (std::size_t (smtk::operation::Observers::*)(smtk::operation::Observers::Key&)) &smtk::operation::Observers::erase)
    .def("insert", (smtk::operation::Observers::Key (smtk::operation::Observers::*)(smtk::operation::Observer, std::string)) &smtk::operation::Observers::insert, pybind11::keep_alive<1, 2>())
    .def("insert", (smtk::operation::Observers::Key (smtk::operation::Observers::*)(smtk::operation::Observer, smtk::operation::Observers::Priority, bool, std::string)) &smtk::operation::Observers::insert, pybind11::keep_alive<1, 2>())
    ;
  py::class_< smtk::operation::Observers::Key >(instance, "Key")
    .def(py::init<>())
    .def("assigned", &smtk::operation::Observers::Key::assigned)
    .def("release", &smtk::operation::Observers::Key::release)
    ;
  return instance;
}

#endif
