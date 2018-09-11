//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_discrete_operators_SplitFaceOperation_h
#define pybind_smtk_session_discrete_operators_SplitFaceOperation_h

#include <pybind11/pybind11.h>

#include "smtk/session/discrete/operators/SplitFaceOperation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::session::discrete::SplitFaceOperation, smtk::operation::Operation > pybind11_init_smtk_session_discrete_SplitFaceOperation(py::module &m)
{
  PySharedPtrClass< smtk::session::discrete::SplitFaceOperation, smtk::operation::Operation > instance(m, "SplitFaceOperation");
  instance
    .def_static("create", (std::shared_ptr<smtk::session::discrete::SplitFaceOperation> (*)()) &smtk::session::discrete::SplitFaceOperation::create)
    .def_static("create", (std::shared_ptr<smtk::session::discrete::SplitFaceOperation> (*)(::std::shared_ptr<smtk::session::discrete::SplitFaceOperation> &)) &smtk::session::discrete::SplitFaceOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::discrete::SplitFaceOperation> (smtk::session::discrete::SplitFaceOperation::*)()) &smtk::session::discrete::SplitFaceOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::discrete::SplitFaceOperation> (smtk::session::discrete::SplitFaceOperation::*)() const) &smtk::session::discrete::SplitFaceOperator::shared_from_this)
    .def("name", &smtk::session::discrete::SplitFaceOperation::name)
    .def("ableToOperate", &smtk::session::discrete::SplitFaceOperation::ableToOperate)
    ;
  return instance;
}

#endif
