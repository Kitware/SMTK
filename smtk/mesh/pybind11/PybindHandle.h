//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_Handle_h
#define pybind_smtk_mesh_Handle_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/Handle.h"

namespace py = pybind11;

void pybind11_init_smtk_mesh_to_json(py::module &m)
{
  m.def("to_json", &smtk::mesh::to_json, "", py::arg("range"));
}

void pybind11_init_smtk_mesh_from_json(py::module &m)
{
  m.def("from_json", &smtk::mesh::from_json, "", py::arg("json"));
}

#endif
