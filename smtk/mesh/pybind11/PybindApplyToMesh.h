//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_utility_ApplyToMesh_h
#define pybind_smtk_mesh_utility_ApplyToMesh_h

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "smtk/mesh/utility/ApplyToMesh.h"

namespace py = pybind11;

inline void pybind11_init_smtk_mesh_utility_applyScalarCellField(py::module &m)
{
  m.def("applyScalarCellField", &smtk::mesh::utility::applyScalarCellField, "", py::arg("arg0"), py::arg("name"), py::arg("ms"));
}

inline void pybind11_init_smtk_mesh_utility_applyScalarPointField(py::module &m)
{
  m.def("applyScalarPointField", &smtk::mesh::utility::applyScalarPointField, "", py::arg("arg0"), py::arg("name"), py::arg("ms"));
}

inline void pybind11_init_smtk_mesh_utility_applyVectorCellField(py::module &m)
{
  m.def("applyVectorCellField", &smtk::mesh::utility::applyVectorCellField, "", py::arg("arg0"), py::arg("name"), py::arg("ms"));
}

inline void pybind11_init_smtk_mesh_utility_applyVectorPointField(py::module &m)
{
  m.def("applyVectorPointField", &smtk::mesh::utility::applyVectorPointField, "", py::arg("arg0"), py::arg("name"), py::arg("ms"));
}

inline void pybind11_init_smtk_mesh_utility_applyWarp(py::module &m)
{
  m.def("applyWarp", &smtk::mesh::utility::applyWarp, "", py::arg("arg0"), py::arg("ms"), py::arg("storePriorCoordinates") = false);
}

inline void pybind11_init_smtk_mesh_utility_undoWarp(py::module &m)
{
  m.def("undoWarp", &smtk::mesh::utility::undoWarp, "", py::arg("ms"));
}

#endif
