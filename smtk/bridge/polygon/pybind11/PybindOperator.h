//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_Operator_h
#define pybind_smtk_bridge_polygon_Operator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/Operator.h"

#include "smtk/operation/XMLOperator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::Operator, smtk::operation::XMLOperator > pybind11_init_smtk_bridge_polygon_Operator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::polygon::Operator, smtk::operation::XMLOperator > instance(m, "Operator");
  instance
    .def("deepcopy", (smtk::bridge::polygon::Operator & (smtk::bridge::polygon::Operator::*)(::smtk::bridge::polygon::Operator const &)) &smtk::bridge::polygon::Operator::operator=)
    ;
  return instance;
}

#endif
