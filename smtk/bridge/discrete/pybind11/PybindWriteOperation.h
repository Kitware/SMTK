//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_WriteOperation_h
#define pybind_smtk_bridge_discrete_operators_WriteOperation_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/WriteOperation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::WriteOperation > pybind11_init_smtk_bridge_discrete_WriteOperation(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::WriteOperation, smtk::operation::Operation > instance(m, "WriteOperation");
  instance
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::WriteOperation> (*)()) &smtk::bridge::discrete::WriteOperation::create)
    .def("ableToOperate", &smtk::bridge::discrete::WriteOperation::ableToOperate)
    ;
  return instance;
}

#endif
