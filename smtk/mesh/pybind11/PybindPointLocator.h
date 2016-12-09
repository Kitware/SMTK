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

#include "smtk/mesh/PointLocator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::PointLocator > pybind11_init_smtk_mesh_PointLocator(py::module &m)
{
  PySharedPtrClass< smtk::mesh::PointLocator > instance(m, "PointLocator");
  instance
    .def(py::init<::smtk::mesh::PointLocator const &>())
    .def(py::init<::smtk::mesh::PointSet const &>())
    .def(py::init<::smtk::mesh::CollectionPtr const, double const * const, ::size_t, bool>())
    .def(py::init<::smtk::mesh::CollectionPtr const, float const * const, ::size_t, bool>())
    .def("deepcopy", (smtk::mesh::PointLocator & (smtk::mesh::PointLocator::*)(::smtk::mesh::PointLocator const &)) &smtk::mesh::PointLocator::operator=)
    .def("find", &smtk::mesh::PointLocator::find, py::arg("x"), py::arg("y"), py::arg("z"), py::arg("radius"), py::arg("results"))
    .def("range", &smtk::mesh::PointLocator::range)
    ;
  return instance;
}

#endif
