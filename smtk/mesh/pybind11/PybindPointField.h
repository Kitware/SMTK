//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_PointField_h
#define pybind_smtk_mesh_PointField_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/PointField.h"

#include "smtk/mesh/PointSet.h"
#include "smtk/mesh/MeshSet.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::PointField > pybind11_init_smtk_mesh_PointField(py::module &m)
{
  PySharedPtrClass< smtk::mesh::PointField > instance(m, "PointField");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::mesh::MeshSet const &, ::std::string const &>())
    .def(py::init<::smtk::mesh::PointField const &>())
    .def("__ne__", (bool (smtk::mesh::PointField::*)(::smtk::mesh::PointField const &) const) &smtk::mesh::PointField::operator!=)
    .def("__lt__", (bool (smtk::mesh::PointField::*)(::smtk::mesh::PointField const &) const) &smtk::mesh::PointField::operator<)
    .def("deepcopy", (smtk::mesh::PointField & (smtk::mesh::PointField::*)(::smtk::mesh::PointField const &)) &smtk::mesh::PointField::operator=)
    .def("__eq__", (bool (smtk::mesh::PointField::*)(::smtk::mesh::PointField const &) const) &smtk::mesh::PointField::operator==)
    .def("points", &smtk::mesh::PointField::points)
    .def("dimension", &smtk::mesh::PointField::dimension)
    .def("get", (::std::vector<double, std::allocator<double> > (smtk::mesh::PointField::*)(::smtk::mesh::HandleRange const &) const) &smtk::mesh::PointField::get, py::arg("pointIds"))
    .def("get", (::std::vector<double, std::allocator<double> > (smtk::mesh::PointField::*)() const) &smtk::mesh::PointField::get)
    .def("get", (bool (smtk::mesh::PointField::*)(::smtk::mesh::HandleRange const &, double *) const) &smtk::mesh::PointField::get, py::arg("pointIds"), py::arg("values"))
    .def("get", (bool (smtk::mesh::PointField::*)(double *) const) &smtk::mesh::PointField::get, py::arg("values"))
    .def("isValid", &smtk::mesh::PointField::isValid)
    .def("meshset", &smtk::mesh::PointField::meshset)
    .def("name", &smtk::mesh::PointField::name)
    .def("set", (bool (smtk::mesh::PointField::*)(::smtk::mesh::HandleRange const &, ::std::vector<double, std::allocator<double> > const &)) &smtk::mesh::PointField::set, py::arg("pointIds"), py::arg("values"))
    .def("set", (bool (smtk::mesh::PointField::*)(::std::vector<double, std::allocator<double> > const &)) &smtk::mesh::PointField::set, py::arg("values"))
    .def("set", (bool (smtk::mesh::PointField::*)(::smtk::mesh::HandleRange const &, double const * const)) &smtk::mesh::PointField::set, py::arg("pointIds"), py::arg("values"))
    .def("set", (bool (smtk::mesh::PointField::*)(double const * const)) &smtk::mesh::PointField::set, py::arg("values"))
    .def("size", &smtk::mesh::PointField::size)
    ;
  return instance;
}

#endif
