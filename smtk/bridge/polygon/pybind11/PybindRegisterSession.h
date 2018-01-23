//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_RegisterSession_h
#define pybind_smtk_bridge_polygon_RegisterSession_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/RegisterSession.h"

namespace py = pybind11;

void pybind11_init__bridge_polygon_registerResources(py::module &m)
{
  m.def("registerResources", &smtk::bridge::polygon::registerResources, "", py::arg("resourceManager"));
}

void pybind11_init__bridge_polygon_registerOperations(py::module &m)
{
  m.def("registerOperations", &smtk::bridge::polygon::registerOperations, "", py::arg("operationManager"));
}

#endif
