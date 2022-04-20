//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_paraview_appcomponents_pqSMTKPythonTrace_h
#define pybind_smtk_extension_paraview_appcomponents_pqSMTKPythonTrace_h

#include <pybind11/pybind11.h>

#include "smtk/extension/paraview/appcomponents/pqSMTKPythonTrace.h"

namespace py = pybind11;

inline py::class_< pqSMTKPythonTrace > pybind11_init_pqSMTKPythonTrace(py::module &m)
{
  py::class_< pqSMTKPythonTrace > instance(m, "pqSMTKPythonTrace");
  instance
    .def(py::init<>())
    .def("traceOperation", &pqSMTKPythonTrace::traceOperation, py::arg("op"), py::arg("testing") = false)
    ;
  return instance;
}

#endif
