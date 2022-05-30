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

#include "smtk/mesh/core/PointField.h"

#include "smtk/mesh/core/PointSet.h"
#include "smtk/mesh/core/MeshSet.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::mesh::PointField > pybind11_init_smtk_mesh_PointField(py::module &m)
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
    .def("type", &smtk::mesh::PointField::type)
    .def("_get_double", [](smtk::mesh::PointField& pf) { std::vector<double> tmp(pf.size() * pf.dimension()); pf.get(tmp.data()); return tmp; })
    .def("_get_double", [](smtk::mesh::PointField& pf, const smtk::mesh::HandleRange& cellIds) { std::vector<double> tmp(cellIds.size() * pf.dimension()); pf.get(cellIds, tmp.data()); return tmp; })
    .def("_get_int", [](smtk::mesh::PointField& pf) { std::vector<int> tmp(pf.size() * pf.dimension()); pf.get(tmp.data()); return tmp; })
    .def("_get_int", [](smtk::mesh::PointField& pf, const smtk::mesh::HandleRange& cellIds) { std::vector<int> tmp(cellIds.size() * pf.dimension()); pf.get(cellIds, tmp.data()); return tmp; })
    .def("isValid", &smtk::mesh::PointField::isValid)
    .def("meshset", &smtk::mesh::PointField::meshset)
    .def("name", &smtk::mesh::PointField::name)
    .def("_set_double", [](smtk::mesh::PointField& pf, const std::vector<double>& data) { return pf.set(data.data()); })
    .def("_set_double", [](smtk::mesh::PointField& pf, const smtk::mesh::HandleRange& cellIds, const std::vector<double>& data) { return pf.set(cellIds, data.data()); })
    .def("_set_int", [](smtk::mesh::PointField& pf, const std::vector<int>& data) { return pf.set(data.data()); })
    .def("_set_int", [](smtk::mesh::PointField& pf, const smtk::mesh::HandleRange& cellIds, const std::vector<int>& data) { return pf.set(cellIds, data.data()); })
    .def("size", &smtk::mesh::PointField::size)
    ;
  return instance;
}

#endif
