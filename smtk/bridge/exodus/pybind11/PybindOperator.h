//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_exodus_Operator_h
#define pybind_smtk_bridge_exodus_Operator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/exodus/Operator.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::exodus::Operator, smtk::model::Operator > pybind11_init_smtk_bridge_exodus_Operator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::exodus::Operator, smtk::model::Operator > instance(m, "Operator");
  instance
    .def("deepcopy", (smtk::bridge::exodus::Operator & (smtk::bridge::exodus::Operator::*)(::smtk::bridge::exodus::Operator const &)) &smtk::bridge::exodus::Operator::operator=)
    ;
  return instance;
}

#endif
