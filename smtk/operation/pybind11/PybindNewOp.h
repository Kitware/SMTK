//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_NewOp_h
#define pybind_smtk_operation_NewOp_h

#include <pybind11/pybind11.h>

#include "smtk/operation/pybind11/PyOperator.h"

#include "smtk/attribute/Attribute.h"

#include "smtk/operation/NewOp.h"

#include "smtk/io/Logger.h"

namespace py = pybind11;

PySharedPtrClass< smtk::operation::NewOp, smtk::operation::PyOperator > pybind11_init_smtk_operation_NewOp(py::module &m)
{
  PySharedPtrClass< smtk::operation::NewOp, smtk::operation::PyOperator > instance(m, "NewOp");
  instance
    .def(py::init<>())
    .def("deepcopy", (smtk::operation::NewOp & (smtk::operation::NewOp::*)(::smtk::operation::NewOp const &)) &smtk::operation::NewOp::operator=)
    .def("classname", &smtk::operation::NewOp::classname)
    .def_static("create", &smtk::operation::PyOperator::create)
    .def("uniqueName", &smtk::operation::NewOp::uniqueName)
    .def("index", &smtk::operation::NewOp::index)
    .def("ableToOperate", &smtk::operation::NewOp::ableToOperate)
    .def("operate", &smtk::operation::NewOp::operate)
    .def("log", &smtk::operation::NewOp::log, pybind11::return_value_policy::reference)
    .def("specification", &smtk::operation::NewOp::specification)
    .def("createBaseSpecification", static_cast<smtk::operation::NewOp::Specification (smtk::operation::NewOp::*)() const>(&smtk::operation::PyOperator::createBaseSpecification))
    .def("parameters", &smtk::operation::NewOp::parameters)
    .def("createResult", &smtk::operation::NewOp::createResult, py::arg("arg0"))
    ;
  py::enum_<smtk::operation::NewOp::Outcome>(instance, "Outcome")
    .value("UNABLE_TO_OPERATE", smtk::operation::NewOp::Outcome::UNABLE_TO_OPERATE)
    .value("CANCELED", smtk::operation::NewOp::Outcome::CANCELED)
    .value("FAILED", smtk::operation::NewOp::Outcome::FAILED)
    .value("SUCCEEDED", smtk::operation::NewOp::Outcome::SUCCEEDED)
    .value("UNKNOWN", smtk::operation::NewOp::Outcome::UNKNOWN)
    .export_values();
  return instance;
}

#endif
