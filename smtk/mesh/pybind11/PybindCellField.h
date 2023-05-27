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
#include <pybind11/stl.h>

#include "smtk/mesh/core/CellField.h"

#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/MeshSet.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::mesh::CellField > pybind11_init_smtk_mesh_CellField(py::module &m)
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
    .def("type", &smtk::mesh::CellField::type)
    .def("_get_double", [](smtk::mesh::CellField& cf) { std::vector<double> tmp(cf.size() * cf.dimension()); cf.get(tmp.data()); return tmp; })
    .def("_get_double", [](smtk::mesh::CellField& cf, const smtk::mesh::HandleRange& cellIds){ std::vector<double> tmp(cellIds.size() * cf.dimension()); cf.get(cellIds, tmp.data()); return tmp; })
    .def("_get_int", [](smtk::mesh::CellField& cf) { std::vector<int> tmp(cf.size() * cf.dimension()); cf.get(tmp.data()); return tmp; })
    .def("_get_int", [](smtk::mesh::CellField& cf, const smtk::mesh::HandleRange& cellIds) { std::vector<int> tmp(cellIds.size() * cf.dimension()); cf.get(cellIds, tmp.data()); return tmp; })
    .def("isValid", &smtk::mesh::CellField::isValid)
    .def("meshset", &smtk::mesh::CellField::meshset)
    .def("name", &smtk::mesh::CellField::name)
    .def("set", [](smtk::mesh::CellField& cf, const std::vector<double>& data) { return cf.set(data.data()); })
    .def("set", [](smtk::mesh::CellField& cf, const smtk::mesh::HandleRange& cellIds, const std::vector<double>& data) { return cf.set(cellIds, data.data()); })
    .def("set", [](smtk::mesh::CellField& cf, const std::vector<int>& data) { return cf.set(data.data()); })
    .def("set", [](smtk::mesh::CellField& cf, const smtk::mesh::HandleRange& cellIds, const std::vector<int>& data) { return cf.set(cellIds, data.data()); })
    .def("size", &smtk::mesh::CellField::size)
    ;
  return instance;
}

#endif
