//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_Operator_h
#define pybind_smtk_bridge_discrete_Operator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/Operator.h"


namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::Operator, smtk::operation::NewOp > pybind11_init_smtk_bridge_discrete_Operator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::Operator, smtk::operation::NewOp > instance(m, "Operator");
  instance
    .def("deepcopy", (smtk::bridge::discrete::Operator & (smtk::bridge::discrete::Operator::*)(::smtk::bridge::discrete::Operator const &)) &smtk::bridge::discrete::Operator::operator=)
    ;
  return instance;
}

#endif
