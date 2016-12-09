//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_moab_HandleRange_h
#define pybind_smtk_mesh_moab_HandleRange_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/moab/HandleRange.h"

namespace py = pybind11;

void pybind11_init_moab_EntityType(py::module &m)
{
  py::enum_<moab::EntityType>(m, "EntityType")
    .value("MBVERTEX", moab::EntityType::MBVERTEX)
    .value("MBEDGE", moab::EntityType::MBEDGE)
    .value("MBTRI", moab::EntityType::MBTRI)
    .value("MBQUAD", moab::EntityType::MBQUAD)
    .value("MBPOLYGON", moab::EntityType::MBPOLYGON)
    .value("MBTET", moab::EntityType::MBTET)
    .value("MBPYRAMID", moab::EntityType::MBPYRAMID)
    .value("MBPRISM", moab::EntityType::MBPRISM)
    .value("MBKNIFE", moab::EntityType::MBKNIFE)
    .value("MBHEX", moab::EntityType::MBHEX)
    .value("MBPOLYHEDRON", moab::EntityType::MBPOLYHEDRON)
    .value("MBENTITYSET", moab::EntityType::MBENTITYSET)
    .value("MBMAXTYPE", moab::EntityType::MBMAXTYPE)
    .export_values();
}

PySharedPtrClass< moab::Range > pybind11_init_moab_Range(py::module &m)
{
  PySharedPtrClass< moab::Range > instance(m, "Range");
  instance
    .def(py::init<>())
    .def(py::init<::moab::Range const &>())
    .def(py::init<::moab::EntityHandle, ::moab::EntityHandle>())
    .def("deepcopy", (moab::Range & (moab::Range::*)(::moab::Range const &)) &moab::Range::operator=)
    .def("__setitem__", (moab::EntityHandle (moab::Range::*)(::moab::EntityID)) &moab::Range::operator[])
    .def("__getitem__", (moab::EntityHandle (moab::Range::*)(::moab::EntityID) const) &moab::Range::operator[])
    .def("all_of_dimension", &moab::Range::all_of_dimension, py::arg("dimension"))
    .def("all_of_type", &moab::Range::all_of_type, py::arg("type"))
    .def("back", &moab::Range::back)
    .def("begin", &moab::Range::begin)
    .def("clear", &moab::Range::clear)
    .def("compactness", &moab::Range::compactness)
    .def("const_pair_begin", &moab::Range::const_pair_begin)
    .def("const_pair_end", &moab::Range::const_pair_end)
    .def("contains", &moab::Range::contains, py::arg("other"))
    .def("empty", &moab::Range::empty)
    .def("end", &moab::Range::end)
    .def("equal_range", &moab::Range::equal_range, py::arg("type"))
    .def("erase", (moab::Range::iterator (moab::Range::*)(::moab::Range::iterator)) &moab::Range::erase, py::arg("iter"))
    .def("erase", (moab::Range::iterator (moab::Range::*)(::moab::Range::iterator, ::moab::Range::iterator)) &moab::Range::erase, py::arg("iter1"), py::arg("iter2"))
    .def("erase", (moab::Range::iterator (moab::Range::*)(::moab::EntityHandle)) &moab::Range::erase, py::arg("val"))
    .def("find", &moab::Range::find, py::arg("val"))
    .def("front", &moab::Range::front)
    .def("get_memory_use", &moab::Range::get_memory_use)
    .def("index", &moab::Range::index, py::arg("handle"))
    .def("insert", (moab::Range::iterator (moab::Range::*)(::moab::Range::iterator, ::moab::EntityHandle)) &moab::Range::insert, py::arg("hint"), py::arg("val"))
    .def("insert", (moab::Range::iterator (moab::Range::*)(::moab::EntityHandle)) &moab::Range::insert, py::arg("val"))
    .def("insert", (moab::Range::iterator (moab::Range::*)(::moab::Range::iterator, ::moab::EntityHandle, ::moab::EntityHandle)) &moab::Range::insert, py::arg("hint"), py::arg("first"), py::arg("last"))
    .def("insert", (moab::Range::iterator (moab::Range::*)(::moab::EntityHandle, ::moab::EntityHandle)) &moab::Range::insert, py::arg("val1"), py::arg("val2"))
    .def("insert", (void (moab::Range::*)(::moab::Range::const_iterator, ::moab::Range::const_iterator)) &moab::Range::insert, py::arg("begin"), py::arg("end"))
    .def_static("lower_bound", (moab::Range::const_iterator (*)(::moab::Range::const_iterator, ::moab::Range::const_iterator, ::moab::EntityHandle)) &moab::Range::lower_bound, py::arg("first"), py::arg("last"), py::arg("val"))
    .def("lower_bound", (moab::Range::const_iterator (moab::Range::*)(::moab::EntityHandle) const) &moab::Range::lower_bound, py::arg("val"))
    .def("lower_bound", (moab::Range::const_iterator (moab::Range::*)(::moab::EntityType) const) &moab::Range::lower_bound, py::arg("type"))
    .def("lower_bound", (moab::Range::const_iterator (moab::Range::*)(::moab::EntityType, ::moab::Range::const_iterator) const) &moab::Range::lower_bound, py::arg("type"), py::arg("first"))
    .def("merge", (void (moab::Range::*)(::moab::Range const &)) &moab::Range::merge, py::arg("range"))
    .def("merge", (void (moab::Range::*)(::moab::Range::const_iterator, ::moab::Range::const_iterator)) &moab::Range::merge, py::arg("beginr"), py::arg("endr"))
    .def("num_of_dimension", &moab::Range::num_of_dimension, py::arg("dim"))
    .def("num_of_type", &moab::Range::num_of_type, py::arg("type"))
    .def("pair_begin", (moab::Range::pair_iterator (moab::Range::*)()) &moab::Range::pair_begin)
    .def("pair_begin", (moab::Range::const_pair_iterator (moab::Range::*)() const) &moab::Range::pair_begin)
    .def("pair_end", (moab::Range::pair_iterator (moab::Range::*)()) &moab::Range::pair_end)
    .def("pair_end", (moab::Range::const_pair_iterator (moab::Range::*)() const) &moab::Range::pair_end)
    .def("pop_back", &moab::Range::pop_back)
    .def("pop_front", &moab::Range::pop_front)
    .def("print", (void (moab::Range::*)(char const *) const) &moab::Range::print, py::arg("indent_prefix") = __null)
    .def("print", (void (moab::Range::*)(::std::ostream &, char const *) const) &moab::Range::print, py::arg("s"), py::arg("indent_prefix") = __null)
    .def("psize", &moab::Range::psize)
    .def("rbegin", &moab::Range::rbegin)
    .def("rend", &moab::Range::rend)
    .def("sanity_check", &moab::Range::sanity_check)
    .def("size", &moab::Range::size)
    .def("subset_by_dimension", &moab::Range::subset_by_dimension, py::arg("dim"))
    .def("subset_by_type", &moab::Range::subset_by_type, py::arg("t"))
    .def("swap", &moab::Range::swap, py::arg("range"))
    .def_static("upper_bound", (moab::Range::const_iterator (*)(::moab::Range::const_iterator, ::moab::Range::const_iterator, ::moab::EntityHandle)) &moab::Range::upper_bound, py::arg("first"), py::arg("last"), py::arg("val"))
    .def("upper_bound", (moab::Range::const_iterator (moab::Range::*)(::moab::EntityHandle) const) &moab::Range::upper_bound, py::arg("val"))
    .def("upper_bound", (moab::Range::const_iterator (moab::Range::*)(::moab::EntityType) const) &moab::Range::upper_bound, py::arg("type"))
    .def("upper_bound", (moab::Range::const_iterator (moab::Range::*)(::moab::EntityType, ::moab::Range::const_iterator) const) &moab::Range::upper_bound, py::arg("type"), py::arg("first"))
    ;
  return instance;
}

PySharedPtrClass< moab::range_base_iter > pybind11_init_moab_range_base_iter(py::module &m)
{
  PySharedPtrClass< moab::range_base_iter > instance(m, "range_base_iter");
  instance
    .def(py::init<>())
    .def(py::init<::moab::range_base_iter const &>())
    .def("deepcopy", (moab::range_base_iter & (moab::range_base_iter::*)(::moab::range_base_iter const &)) &moab::range_base_iter::operator=)
    ;
  return instance;
}

PySharedPtrClass< moab::range_inserter > pybind11_init_moab_range_inserter(py::module &m)
{
  PySharedPtrClass< moab::range_inserter > instance(m, "range_inserter");
  instance
    .def(py::init<::moab::range_inserter const &>())
    .def(py::init<::moab::Range &>())
    .def("__mul__", (moab::range_inserter & (moab::range_inserter::*)()) &moab::range_inserter::operator*)
    .def("deepcopy", (moab::range_inserter & (moab::range_inserter::*)(::moab::range_inserter const &)) &moab::range_inserter::operator=)
    .def("deepcopy", (moab::range_inserter & (moab::range_inserter::*)(::moab::Range::value_type const &)) &moab::range_inserter::operator=)
    ;
  return instance;
}

PySharedPtrClass< moab::range_iter_tag > pybind11_init_moab_range_iter_tag(py::module &m)
{
  PySharedPtrClass< moab::range_iter_tag > instance(m, "range_iter_tag");
  instance
    .def(py::init<>())
    .def(py::init<::moab::range_iter_tag const &>())
    .def("deepcopy", (moab::range_iter_tag & (moab::range_iter_tag::*)(::moab::range_iter_tag const &)) &moab::range_iter_tag::operator=)
    ;
  return instance;
}

PySharedPtrClass< std::bidirectional_iterator_tag > pybind11_init_std_bidirectional_iterator_tag(py::module &m)
{
  PySharedPtrClass< std::bidirectional_iterator_tag > instance(m, "bidirectional_iterator_tag");
  instance
    .def(py::init<>())
    .def(py::init<::std::bidirectional_iterator_tag const &>())
    .def("deepcopy", (std::bidirectional_iterator_tag & (std::bidirectional_iterator_tag::*)(::std::bidirectional_iterator_tag const &)) &std::bidirectional_iterator_tag::operator=)
    ;
  return instance;
}

#endif
