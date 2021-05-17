//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_interpolation_PointCloud_h
#define pybind_smtk_mesh_interpolation_PointCloud_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/interpolation/PointCloud.h"

namespace py = pybind11;

inline py::class_< smtk::mesh::PointCloud > pybind11_init_smtk_mesh_PointCloud(py::module &m)
{
  py::class_< smtk::mesh::PointCloud > instance(m, "PointCloud");
  instance
    .def(py::init<::size_t, ::std::function<std::array<double, 3> (unsigned long)> const &, ::std::function<double (unsigned long)> const &, ::std::function<bool (unsigned long)> const &>())
    .def(py::init<>())
    .def(py::init<::size_t, ::std::function<std::array<double, 3> (unsigned long)> const &, ::std::function<double (unsigned long)> const &>())
    .def(py::init<::size_t, double const * const, double const * const>())
    .def(py::init<::size_t, float const * const, float const * const>())
    .def(py::init<::smtk::mesh::PointCloud const &>())
    .def("deepcopy", (smtk::mesh::PointCloud & (smtk::mesh::PointCloud::*)(::smtk::mesh::PointCloud const &)) &smtk::mesh::PointCloud::operator=)
    .def("size", &smtk::mesh::PointCloud::size)
    .def("coordinates", &smtk::mesh::PointCloud::coordinates)
    .def("data", &smtk::mesh::PointCloud::data)
    ;
  return instance;
}

#endif
