//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_discrete_operators_CreateEdgesOperation_h
#define pybind_smtk_session_discrete_operators_CreateEdgesOperation_h

#include <pybind11/pybind11.h>

#include "smtk/session/discrete/operators/CreateEdgesOperation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::session::discrete::CreateEdgesOperation, smtk::operation::Operation > pybind11_init_smtk_session_discrete_CreateEdgesOperation(py::module &m)
{
  PySharedPtrClass< smtk::session::discrete::CreateEdgesOperation, smtk::operation::Operation > instance(m, "CreateEdgesOperation");
  instance
    .def_static("create", (std::shared_ptr<smtk::session::discrete::CreateEdgesOperation> (*)()) &smtk::session::discrete::CreateEdgesOperation::create)
    .def("ableToOperate", &smtk::session::discrete::CreateEdgesOperation::ableToOperate)
    ;
  return instance;
}

#endif
