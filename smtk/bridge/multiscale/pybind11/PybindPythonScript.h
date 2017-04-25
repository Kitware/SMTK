//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_multiscale_operators_PythonScript_h
#define pybind_smtk_bridge_multiscale_operators_PythonScript_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/multiscale/operators/PythonScript.h"

#include "smtk/bridge/multiscale/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::multiscale::PythonScript, smtk::bridge::multiscale::Operator > pybind11_init_smtk_bridge_multiscale_PythonScript(py::module &m)
{
  PySharedPtrClass< smtk::bridge::multiscale::PythonScript, smtk::bridge::multiscale::Operator > instance(m, "PythonScript");
  instance
    .def("deepcopy", (smtk::bridge::multiscale::PythonScript & (smtk::bridge::multiscale::PythonScript::*)(::smtk::bridge::multiscale::PythonScript const &)) &smtk::bridge::multiscale::PythonScript::operator=)
    ;
  return instance;
}

#endif
