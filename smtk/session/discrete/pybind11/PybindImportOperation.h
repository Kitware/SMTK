//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_discrete_operators_ImportOperation_h
#define pybind_smtk_session_discrete_operators_ImportOperation_h

#include <pybind11/pybind11.h>

#include "smtk/session/discrete/operators/ImportOperation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::session::discrete::ImportOperation, smtk::operation::Operation > pybind11_init_smtk_session_discrete_ImportOperation(py::module &m)
{
  PySharedPtrClass< smtk::session::discrete::ImportOperation, smtk::operation::Operation > instance(m, "ImportOperation");
  instance
    .def_static("create", (std::shared_ptr<smtk::session::discrete::ImportOperation> (*)()) &smtk::session::discrete::ImportOperation::create)
    .def_static("create", (std::shared_ptr<smtk::session::discrete::ImportOperation> (*)(::std::shared_ptr<smtk::session::discrete::ImportOperation> &)) &smtk::session::discrete::ImportOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::discrete::ImportOperation> (smtk::session::discrete::ImportOperation::*)()) &smtk::session::discrete::ImportOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::discrete::ImportOperation> (smtk::session::discrete::ImportOperation::*)() const) &smtk::session::discrete::ImportOperator::shared_from_this)
    .def("name", &smtk::session::discrete::ImportOperation::name)
    .def("ableToOperate", &smtk::session::discrete::ImportOperation::ableToOperate)
    ;
  return instance;
}

#endif
