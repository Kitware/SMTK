//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_PointLocator_h
#define pybind_smtk_mesh_PointLocator_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/core/PointLocator.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::mesh::PointLocator > pybind11_init_smtk_mesh_PointLocator(py::module &m)
{
  PySharedPtrClass< smtk::mesh::PointLocator > instance(m, "PointLocator");
  instance
    .def(py::init<::smtk::mesh::PointLocator const &>())
    .def(py::init<::smtk::mesh::PointSet const &>())
    .def(py::init<::smtk::mesh::ResourcePtr const, ::size_t, const std::function<std::array<double, 3>(::size_t)>&>())
    .def(py::init<::smtk::mesh::ResourcePtr const, ::size_t, double const * const>())
    .def(py::init<::smtk::mesh::ResourcePtr const, ::size_t, float const * const>())
    .def("deepcopy", (smtk::mesh::PointLocator & (smtk::mesh::PointLocator::*)(::smtk::mesh::PointLocator const &)) &smtk::mesh::PointLocator::operator=)
    .def("find", &smtk::mesh::PointLocator::find, py::arg("x"), py::arg("y"), py::arg("z"), py::arg("radius"), py::arg("results"))
    .def("range", &smtk::mesh::PointLocator::range)
    ;
  return instance;
}

#endif
