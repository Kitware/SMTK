//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_discrete_operators_GrowOperation_h
#define pybind_smtk_session_discrete_operators_GrowOperation_h

#include <pybind11/pybind11.h>

#include "smtk/session/discrete/operators/GrowOperation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::session::discrete::GrowOperation, smtk::operation::Operation > pybind11_init_smtk_session_discrete_GrowOperation(py::module &m)
{
  PySharedPtrClass< smtk::session::discrete::GrowOperation, smtk::operation::Operation > instance(m, "GrowOperation");
  instance
    .def_static("create", (std::shared_ptr<smtk::session::discrete::GrowOperation> (*)()) &smtk::session::discrete::GrowOperation::create)
    .def_static("create", (std::shared_ptr<smtk::session::discrete::GrowOperation> (*)(::std::shared_ptr<smtk::session::discrete::GrowOperation> &)) &smtk::session::discrete::GrowOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::discrete::GrowOperation> (smtk::session::discrete::GrowOperation::*)()) &smtk::session::discrete::GrowOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::discrete::GrowOperation> (smtk::session::discrete::GrowOperation::*)() const) &smtk::session::discrete::GrowOperator::shared_from_this)
    .def("name", &smtk::session::discrete::GrowOperation::name)
    .def("ableToOperate", &smtk::session::discrete::GrowOperation::ableToOperate)
    ;
  return instance;
}

#endif
