//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_mesh_Operator_h
#define pybind_smtk_bridge_mesh_Operator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/mesh/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::mesh::Operator, smtk::model::Operator > pybind11_init_smtk_bridge_mesh_Operator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::mesh::Operator, smtk::model::Operator > instance(m, "Operator");
  instance
    .def("deepcopy", (smtk::bridge::mesh::Operator & (smtk::bridge::mesh::Operator::*)(::smtk::bridge::mesh::Operator const &)) &smtk::bridge::mesh::Operator::operator=)
    ;
  return instance;
}

#endif
