//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_Operation_h
#define pybind_smtk_operation_Operation_h

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

#include "smtk/operation/pybind11/PyOperation.h"

#include "smtk/attribute/Attribute.h"

#include "smtk/operation/Operation.h"

#include "smtk/io/Logger.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::operation::Operation, smtk::operation::PyOperation > pybind11_init_smtk_operation_Operation(py::module &m)
{
  PySharedPtrClass< smtk::operation::Operation, smtk::operation::PyOperation > instance(m, "Operation");

  py::enum_<smtk::operation::Operation::ObserverOption>(instance, "ObserverOption")
    .value("InvokeObservers", smtk::operation::Operation::ObserverOption::InvokeObservers)
    .value("SkipObservers", smtk::operation::Operation::ObserverOption::SkipObservers)
    .export_values();

  py::enum_<smtk::operation::Operation::LockOption>(instance, "LockOption")
    .value("LockAll", smtk::operation::Operation::LockOption::LockAll)
    .value("ParentLocksOnly", smtk::operation::Operation::LockOption::ParentLocksOnly)
    .value("SkipLocks", smtk::operation::Operation::LockOption::SkipLocks)
    .export_values();

  py::enum_<smtk::operation::Operation::ParametersOption>(instance, "ParametersOption")
    .value("Validate", smtk::operation::Operation::ParametersOption::Validate)
    .value("SkipValidation", smtk::operation::Operation::ParametersOption::SkipValidation)
    .export_values();

  instance
    .def(py::init<>())
    .def("deepcopy", (smtk::operation::Operation & (smtk::operation::Operation::*)(::smtk::operation::Operation const &)) &smtk::operation::Operation::operator=)
    .def_static("create", &smtk::operation::PyOperation::create)
    .def("typeName", &smtk::operation::Operation::typeName)
    .def("index", &smtk::operation::Operation::index)
    .def("ableToOperate", [](smtk::operation::Operation& self)
      {
        if (auto* pyself = dynamic_cast<smtk::operation::PyOperation*>(&self))
        {
          // Release the GIL as ableToOperate will re-acquire it.
          pybind11::gil_scoped_release thread_state(true);
          return pyself->ableToOperate();
        }
        return self.ableToOperate();
      }
    )
    .def("operate", [](smtk::operation::Operation& self)
      {
        if (auto* pyself = dynamic_cast<smtk::operation::PyOperation*>(&self))
        {
          // Release the GIL as operate will re-acquire it.
          pybind11::gil_scoped_release thread_state(true);
          return pyself->operate();
        }
        return self.operate();
      }
    )
    .def("safeOperate", (smtk::operation::Operation::Outcome (smtk::operation::Operation::*)()) &smtk::operation::Operation::safeOperate)
    .def("safeOperate", (smtk::operation::Operation::Outcome (smtk::operation::Operation::*)(smtk::operation::Handler)) &smtk::operation::Operation::safeOperate)
    .def("log", [](smtk::operation::Operation& self)
      {
        if (auto* pyself = dynamic_cast<smtk::operation::PyOperation*>(&self))
        {
          // Release the GIL as ableToOperate will re-acquire it.
          pybind11::gil_scoped_release thread_state(true);
          return pyself->log();
        }
        return self.log();
      }, pybind11::return_value_policy::reference
    )
    .def("specification", [](smtk::operation::Operation& self)
      {
        if (auto* pyself = dynamic_cast<smtk::operation::PyOperation*>(&self))
        {
          // Release the GIL as specification() will re-acquire it.
          pybind11::gil_scoped_release thread_state(true);
          return pyself->specification();
        }
        return self.specification();
      }
    )
    .def("createBaseSpecification", static_cast<smtk::operation::Operation::Specification (smtk::operation::Operation::*)() const>(&smtk::operation::PyOperation::createBaseSpecification))
    .def("_parameters", (smtk::operation::Operation::Parameters (smtk::operation::Operation::*)()) &smtk::operation::Operation::parameters)
    .def("createResult", &smtk::operation::Operation::createResult, py::arg("arg0"))
    .def("manager", &smtk::operation::Operation::manager)
    .def("managers", &smtk::operation::Operation::managers)
    .def("restoreTrace", (bool (smtk::operation::Operation::*)(::std::string const &)) &smtk::operation::Operation::restoreTrace)
    ;
  py::enum_<smtk::operation::Operation::Outcome>(instance, "Outcome")
    .value("UNABLE_TO_OPERATE", smtk::operation::Operation::Outcome::UNABLE_TO_OPERATE)
    .value("CANCELED", smtk::operation::Operation::Outcome::CANCELED)
    .value("FAILED", smtk::operation::Operation::Outcome::FAILED)
    .value("SUCCEEDED", smtk::operation::Operation::Outcome::SUCCEEDED)
    .value("UNKNOWN", smtk::operation::Operation::Outcome::UNKNOWN)
    .export_values();
  return instance;
}

#endif
