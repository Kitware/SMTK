//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_interpolation_StructuredGrid_h
#define pybind_smtk_mesh_interpolation_StructuredGrid_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/interpolation/StructuredGrid.h"

namespace py = pybind11;

py::class_< smtk::mesh::StructuredGrid > pybind11_init_smtk_mesh_StructuredGrid(py::module &m)
{
  py::class_< smtk::mesh::StructuredGrid > instance(m, "StructuredGrid");
  instance
    .def(py::init<>())
    .def(py::init<int const *, double const *, double const *, ::std::function<double (int, int)> const &, ::std::function<bool (int, int)> const &>())
    .def(py::init<int const *, double const *, double const *, ::std::function<double (int, int)> const &>())
    .def(py::init<::smtk::mesh::StructuredGrid const &>())
    .def("deepcopy", (smtk::mesh::StructuredGrid & (smtk::mesh::StructuredGrid::*)(::smtk::mesh::StructuredGrid const &)) &smtk::mesh::StructuredGrid::operator=)
    .def("containsIndex", &smtk::mesh::StructuredGrid::containsIndex, py::arg("ix"), py::arg("iy"))
    .def("data", &smtk::mesh::StructuredGrid::data)
    .def("size", &smtk::mesh::StructuredGrid::size)
    .def_readwrite("m_extent", &smtk::mesh::StructuredGrid::m_extent)
    .def_readwrite("m_origin", &smtk::mesh::StructuredGrid::m_origin)
    .def_readwrite("m_spacing", &smtk::mesh::StructuredGrid::m_spacing)
    ;
  return instance;
}

#endif
