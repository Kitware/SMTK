//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_Operation_h
#define pybind_smtk_bridge_discrete_Operation_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/Operation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::Operation, smtk::operation::Operation > pybind11_init_smtk_bridge_discrete_Operation(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::Operation, smtk::operation::Operation > instance(m, "Operation");
  instance
    .def("deepcopy", (smtk::bridge::discrete::Operation & (smtk::bridge::discrete::Operation::*)(::smtk::bridge::discrete::Operator const &)) &smtk::bridge::discrete::Operator::operator=)
    ;
  return instance;
}

#endif
