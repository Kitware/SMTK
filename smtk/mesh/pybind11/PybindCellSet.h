//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_CellSet_h
#define pybind_smtk_mesh_CellSet_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/CellSet.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::CellSet > pybind11_init_smtk_mesh_CellSet(py::module &m)
{
  PySharedPtrClass< smtk::mesh::CellSet > instance(m, "CellSet");
  instance
    .def(py::init<::smtk::mesh::CollectionPtr const &, ::smtk::mesh::HandleRange const &>())
    .def(py::init<::smtk::mesh::ConstCollectionPtr const &, ::smtk::mesh::HandleRange const &>())
    .def(py::init<::smtk::mesh::CollectionPtr const &, ::std::vector<unsigned long, std::allocator<unsigned long> > const &>())
    .def(py::init<::smtk::mesh::CollectionPtr const &, ::std::set<unsigned long, std::less<unsigned long>, std::allocator<unsigned long> > const &>())
    .def(py::init<::smtk::mesh::CellSet const &>())
    .def("__ne__", (bool (smtk::mesh::CellSet::*)(::smtk::mesh::CellSet const &) const) &smtk::mesh::CellSet::operator!=)
    .def("deepcopy", (smtk::mesh::CellSet & (smtk::mesh::CellSet::*)(::smtk::mesh::CellSet const &)) &smtk::mesh::CellSet::operator=)
    .def("__eq__", (bool (smtk::mesh::CellSet::*)(::smtk::mesh::CellSet const &) const) &smtk::mesh::CellSet::operator==)
    .def("append", &smtk::mesh::CellSet::append, py::arg("other"))
    .def("collection", &smtk::mesh::CellSet::collection)
    .def("is_empty", &smtk::mesh::CellSet::is_empty)
    .def("pointConnectivity", (smtk::mesh::PointConnectivity (smtk::mesh::CellSet::*)() const) &smtk::mesh::CellSet::pointConnectivity)
    .def("pointConnectivity", (smtk::mesh::PointConnectivity (smtk::mesh::CellSet::*)(::size_t) const) &smtk::mesh::CellSet::pointConnectivity, py::arg("arg0"))
    .def("points", (smtk::mesh::PointSet (smtk::mesh::CellSet::*)() const) &smtk::mesh::CellSet::points)
    .def("points", (smtk::mesh::PointSet (smtk::mesh::CellSet::*)(::size_t) const) &smtk::mesh::CellSet::points, py::arg("arg0"))
    .def("range", &smtk::mesh::CellSet::range)
    .def("size", &smtk::mesh::CellSet::size)
    .def("types", &smtk::mesh::CellSet::types)
    ;
  return instance;
}

void pybind11_init_smtk_mesh_cell_for_each(py::module &m)
{
  m.def("for_each", (void (*)(const smtk::mesh::CellSet&, smtk::mesh::CellForEach&)) &smtk::mesh::for_each, "", py::arg("a"), py::arg("filter"));
}

void pybind11_init_smtk_mesh_cell_point_difference(py::module &m)
{
  m.def("point_difference", (smtk::mesh::CellSet (*)(const smtk::mesh::CellSet&, const smtk::mesh::CellSet&, smtk::mesh::ContainmentType)) &smtk::mesh::point_difference, "", py::arg("a"), py::arg("b"), py::arg("t"));
}

void pybind11_init_smtk_mesh_cell_point_intersect(py::module &m)
{
  m.def("point_intersect", (smtk::mesh::CellSet (*)(const smtk::mesh::CellSet&, const smtk::mesh::CellSet&, smtk::mesh::ContainmentType)) &smtk::mesh::point_intersect, "", py::arg("a"), py::arg("b"), py::arg("t"));
}

void pybind11_init_smtk_mesh_cell_set_difference(py::module &m)
{
  m.def("set_difference", (smtk::mesh::CellSet (*)(const smtk::mesh::CellSet&, const smtk::mesh::CellSet&)) &smtk::mesh::set_difference, "", py::arg("a"), py::arg("b"));
}

void pybind11_init_smtk_mesh_cell_set_intersect(py::module &m)
{
  m.def("set_intersect", (smtk::mesh::CellSet (*)(const smtk::mesh::CellSet&, const smtk::mesh::CellSet&)) &smtk::mesh::set_intersect, "", py::arg("a"), py::arg("b"));
}

void pybind11_init_smtk_mesh_cell_set_union(py::module &m)
{
  m.def("set_union", (smtk::mesh::CellSet (*)(const smtk::mesh::CellSet&, const smtk::mesh::CellSet&)) &smtk::mesh::set_union, "", py::arg("a"), py::arg("b"));
}

#endif
