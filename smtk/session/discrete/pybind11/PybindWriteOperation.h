//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_discrete_operators_WriteOperation_h
#define pybind_smtk_session_discrete_operators_WriteOperation_h

#include <pybind11/pybind11.h>

#include "smtk/session/discrete/operators/WriteOperation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::session::discrete::WriteOperation > pybind11_init_smtk_session_discrete_WriteOperation(py::module &m)
{
  PySharedPtrClass< smtk::session::discrete::WriteOperation, smtk::operation::Operation > instance(m, "WriteOperation");
  instance
    .def_static("create", (std::shared_ptr<smtk::session::discrete::WriteOperation> (*)()) &smtk::session::discrete::WriteOperation::create)
    .def("ableToOperate", &smtk::session::discrete::WriteOperation::ableToOperate)
    ;
  return std::move(instance);
}

#endif
