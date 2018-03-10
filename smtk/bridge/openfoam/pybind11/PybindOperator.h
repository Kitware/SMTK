//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_openfoam_Operator_h
#define pybind_smtk_bridge_openfoam_Operator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/openfoam/Operator.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::openfoam::Operator, smtk::model::Operator > pybind11_init_smtk_bridge_openfoam_Operator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::openfoam::Operator, smtk::model::Operator > instance(m, "Operator");
  instance
    .def("deepcopy", (smtk::bridge::openfoam::Operator & (smtk::bridge::openfoam::Operator::*)(::smtk::bridge::openfoam::Operator const &)) &smtk::bridge::openfoam::Operator::operator=)
    ;
  return instance;
}

#endif
