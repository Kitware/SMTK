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

#include "smtk/attribute/Attribute.h"

#include "smtk/operation/Observer.h"

namespace py = pybind11;

void pybind11_init_smtk_operation_EventType(py::module &m)
{
  py::enum_<smtk::operation::EventType>(m, "EventType")
    .value("CREATED", smtk::operation::EventType::CREATED)
    .value("WILL_OPERATE", smtk::operation::EventType::WILL_OPERATE)
    .value("DID_OPERATE", smtk::operation::EventType::DID_OPERATE)
    .export_values();
}

py::class_< smtk::operation::Observers > pybind11_init_smtk_operation_Observers(py::module &m)
{
  py::class_< smtk::operation::Observers > instance(m, "Observers");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::operation::Observers const &>())
    .def("__call__", (int (smtk::operation::Observers::*)(::std::shared_ptr<smtk::operation::NewOp>, ::smtk::operation::EventType, ::smtk::operation::NewOp::Result)) &smtk::operation::Observers::operator())
    .def("deepcopy", (smtk::operation::Observers & (smtk::operation::Observers::*)(::smtk::operation::Observers const &)) &smtk::operation::Observers::operator=)
    .def("erase", &smtk::operation::Observers::erase, py::arg("arg0"))
    .def("insert", &smtk::operation::Observers::insert, py::arg("arg0"))
    ;
  return instance;
}

#endif
