//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_PointSet_h
#define pybind_smtk_mesh_PointSet_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/mesh/PointSet.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::PointSet > pybind11_init_smtk_mesh_PointSet(py::module &m)
{
  PySharedPtrClass< smtk::mesh::PointSet > instance(m, "PointSet");
  instance
    .def(py::init<::smtk::mesh::CollectionPtr const &, ::smtk::mesh::HandleRange const &>())
    .def(py::init<::smtk::mesh::ConstCollectionPtr const &, ::smtk::mesh::HandleRange const &>())
    .def(py::init<::smtk::mesh::CollectionPtr const &, ::std::vector<smtk::mesh::Handle, std::allocator<smtk::mesh::Handle> > const &>())
    .def(py::init<::smtk::mesh::CollectionPtr const &, ::std::set<smtk::mesh::Handle, std::less<smtk::mesh::Handle>, std::allocator<smtk::mesh::Handle> > const &>())
    .def(py::init<::smtk::mesh::PointSet const &>())
    .def("__ne__", (bool (smtk::mesh::PointSet::*)(::smtk::mesh::PointSet const &) const) &smtk::mesh::PointSet::operator!=)
    .def("deepcopy", (smtk::mesh::PointSet & (smtk::mesh::PointSet::*)(::smtk::mesh::PointSet const &)) &smtk::mesh::PointSet::operator=)
    .def("__eq__", (bool (smtk::mesh::PointSet::*)(::smtk::mesh::PointSet const &) const) &smtk::mesh::PointSet::operator==)
    .def("collection", &smtk::mesh::PointSet::collection)
    .def("contains", &smtk::mesh::PointSet::contains, py::arg("pointId"))
    .def("find", &smtk::mesh::PointSet::find, py::arg("pointId"))
    .def("get", (bool (smtk::mesh::PointSet::*)(::std::vector<double, std::allocator<double> > &) const) &smtk::mesh::PointSet::get, py::arg("xyz"))
    .def("get", (bool (smtk::mesh::PointSet::*)(double *) const) &smtk::mesh::PointSet::get, py::arg("xyz"))
    .def("get", (bool (smtk::mesh::PointSet::*)(float *) const) &smtk::mesh::PointSet::get, py::arg("xyz"))
    .def("get", (bool (smtk::mesh::PointSet::*)(::std::vector<float, std::allocator<float> > &) const) &smtk::mesh::PointSet::get, py::arg("xyz"))
    .def("is_empty", &smtk::mesh::PointSet::is_empty)
    .def("numberOfPoints", &smtk::mesh::PointSet::numberOfPoints)
    .def("range", &smtk::mesh::PointSet::range)
    .def("set", (bool (smtk::mesh::PointSet::*)(::std::vector<double, std::allocator<double> > const &) const) &smtk::mesh::PointSet::set, py::arg("xyz"))
    .def("set", (bool (smtk::mesh::PointSet::*)(double const * const) const) &smtk::mesh::PointSet::set, py::arg("xyz"))
    .def("set", (bool (smtk::mesh::PointSet::*)(float const * const)) &smtk::mesh::PointSet::set, py::arg("xyz"))
    .def("set", (bool (smtk::mesh::PointSet::*)(::std::vector<float, std::allocator<float> > const &)) &smtk::mesh::PointSet::set, py::arg("xyz"))
    .def("size", &smtk::mesh::PointSet::size)
    ;
  return instance;
}

void pybind11_init_smtk_mesh_point_for_each(py::module &m)
{
  m.def("for_each", (void (*)(const smtk::mesh::PointSet&, smtk::mesh::PointForEach&)) &smtk::mesh::for_each, "", py::arg("a"), py::arg("filter"));
}

void pybind11_init_smtk_mesh_point_set_difference(py::module &m)
{
  m.def("set_difference", (smtk::mesh::PointSet (*)(const smtk::mesh::PointSet&, const smtk::mesh::PointSet&)) &smtk::mesh::set_difference, "", py::arg("a"), py::arg("b"));
}

void pybind11_init_smtk_mesh_point_set_intersect(py::module &m)
{
  m.def("set_intersect", (smtk::mesh::PointSet (*)(const smtk::mesh::PointSet&, const smtk::mesh::PointSet&)) &smtk::mesh::set_intersect, "", py::arg("a"), py::arg("b"));
}

void pybind11_init_smtk_mesh_point_set_union(py::module &m)
{
  m.def("set_union", (smtk::mesh::PointSet (*)(const smtk::mesh::PointSet&, const smtk::mesh::PointSet&)) &smtk::mesh::set_union, "", py::arg("a"), py::arg("b"));
}

#endif
