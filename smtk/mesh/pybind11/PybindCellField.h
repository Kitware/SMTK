//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_CellField_h
#define pybind_smtk_mesh_CellField_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/CellField.h"

#include "smtk/mesh/CellSet.h"
#include "smtk/mesh/MeshSet.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::CellField > pybind11_init_smtk_mesh_CellField(py::module &m)
{
  PySharedPtrClass< smtk::mesh::CellField > instance(m, "CellField");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::mesh::MeshSet const &, ::std::string const &>())
    .def(py::init<::smtk::mesh::CellField const &>())
    .def("__ne__", (bool (smtk::mesh::CellField::*)(::smtk::mesh::CellField const &) const) &smtk::mesh::CellField::operator!=)
    .def("__lt__", (bool (smtk::mesh::CellField::*)(::smtk::mesh::CellField const &) const) &smtk::mesh::CellField::operator<)
    .def("deepcopy", (smtk::mesh::CellField & (smtk::mesh::CellField::*)(::smtk::mesh::CellField const &)) &smtk::mesh::CellField::operator=)
    .def("__eq__", (bool (smtk::mesh::CellField::*)(::smtk::mesh::CellField const &) const) &smtk::mesh::CellField::operator==)
    .def("cells", &smtk::mesh::CellField::cells)
    .def("dimension", &smtk::mesh::CellField::dimension)
    .def("get", (::std::vector<double, std::allocator<double> > (smtk::mesh::CellField::*)(::smtk::mesh::HandleRange const &) const) &smtk::mesh::CellField::get, py::arg("cellIds"))
    .def("get", (::std::vector<double, std::allocator<double> > (smtk::mesh::CellField::*)() const) &smtk::mesh::CellField::get)
    .def("get", (bool (smtk::mesh::CellField::*)(::smtk::mesh::HandleRange const &, double *) const) &smtk::mesh::CellField::get, py::arg("cellIds"), py::arg("values"))
    .def("get", (bool (smtk::mesh::CellField::*)(double *) const) &smtk::mesh::CellField::get, py::arg("values"))
    .def("isValid", &smtk::mesh::CellField::isValid)
    .def("meshset", &smtk::mesh::CellField::meshset)
    .def("name", &smtk::mesh::CellField::name)
    .def("set", (bool (smtk::mesh::CellField::*)(::smtk::mesh::HandleRange const &, ::std::vector<double, std::allocator<double> > const &)) &smtk::mesh::CellField::set, py::arg("cellIds"), py::arg("values"))
    .def("set", (bool (smtk::mesh::CellField::*)(::std::vector<double, std::allocator<double> > const &)) &smtk::mesh::CellField::set, py::arg("values"))
    .def("set", (bool (smtk::mesh::CellField::*)(::smtk::mesh::HandleRange const &, double const * const)) &smtk::mesh::CellField::set, py::arg("cellIds"), py::arg("values"))
    .def("set", (bool (smtk::mesh::CellField::*)(double const * const)) &smtk::mesh::CellField::set, py::arg("values"))
    .def("size", &smtk::mesh::CellField::size)
    ;
  return instance;
}

#endif
