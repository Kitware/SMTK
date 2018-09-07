//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_discrete_operators_MergeOperation_h
#define pybind_smtk_session_discrete_operators_MergeOperation_h

#include <pybind11/pybind11.h>

#include "smtk/session/discrete/operators/MergeOperation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::session::discrete::MergeOperation, smtk::operation::Operation > pybind11_init_smtk_session_discrete_MergeOperation(py::module &m)
{
  PySharedPtrClass< smtk::session::discrete::MergeOperation, smtk::operation::Operation > instance(m, "MergeOperation");
  instance
    .def_static("create", (std::shared_ptr<smtk::session::discrete::MergeOperation> (*)()) &smtk::session::discrete::MergeOperation::create)
    .def_static("create", (std::shared_ptr<smtk::session::discrete::MergeOperation> (*)(::std::shared_ptr<smtk::session::discrete::MergeOperation> &)) &smtk::session::discrete::MergeOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::discrete::MergeOperation> (smtk::session::discrete::MergeOperation::*)()) &smtk::session::discrete::MergeOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::discrete::MergeOperation> (smtk::session::discrete::MergeOperation::*)() const) &smtk::session::discrete::MergeOperator::shared_from_this)
    .def("name", &smtk::session::discrete::MergeOperation::name)
    .def("ableToOperate", &smtk::session::discrete::MergeOperation::ableToOperate)
    ;
  return instance;
}

#endif
