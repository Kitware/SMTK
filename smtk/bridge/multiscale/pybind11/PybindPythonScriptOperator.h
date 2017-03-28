//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_multiscale_operators_PythonScriptOperator_h
#define pybind_smtk_bridge_multiscale_operators_PythonScriptOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/multiscale/operators/PythonScriptOperator.h"

#include "smtk/bridge/multiscale/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::multiscale::PythonScriptOperator, smtk::bridge::multiscale::Operator > pybind11_init_smtk_bridge_multiscale_PythonScriptOperator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::multiscale::PythonScriptOperator, smtk::bridge::multiscale::Operator > instance(m, "PythonScriptOperator");
  instance
    .def("deepcopy", (smtk::bridge::multiscale::PythonScriptOperator & (smtk::bridge::multiscale::PythonScriptOperator::*)(::smtk::bridge::multiscale::PythonScriptOperator const &)) &smtk::bridge::multiscale::PythonScriptOperator::operator=)
    ;
  return instance;
}

#endif
