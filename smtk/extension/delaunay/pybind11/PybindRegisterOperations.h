//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_delaunay_RegisterOperations_h
#define pybind_smtk_extension_delaunay_RegisterOperations_h

#include <pybind11/pybind11.h>

#include "smtk/extension/delaunay/operators/RegisterOperations.h"

namespace py = pybind11;

void pybind11_init__extension_delaunay_registerOperations(py::module &m)
{
  m.def("registerOperations", &smtk::extension::delaunay::registerOperations, "", py::arg("operationManager"));
}

#endif
