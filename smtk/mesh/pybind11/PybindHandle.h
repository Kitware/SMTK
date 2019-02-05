//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_core_Handle_h
#define pybind_smtk_mesh_core_Handle_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/core/Handle.h"

namespace py = pybind11;

py::class_< smtk::mesh::HandleInterval > pybind11_init_HandleInterval(py::module &m)
{
  py::class_< smtk::mesh::HandleInterval > instance(m, "HandleInterval");
  instance
    .def(py::init<smtk::mesh::HandleInterval const &>())
    .def(py::init<>())
    .def(py::init<long unsigned int const &>())
    .def(py::init<long unsigned int const &, long unsigned int const &>())
    .def("deepcopy", (smtk::mesh::HandleInterval & (smtk::mesh::HandleInterval::*)(smtk::mesh::HandleInterval const &)) &smtk::mesh::HandleInterval::operator=)
    .def("first", &smtk::mesh::HandleInterval::first)
    .def("last", &smtk::mesh::HandleInterval::last)
    .def("lower", &smtk::mesh::HandleInterval::lower)
    .def("upper", &smtk::mesh::HandleInterval::upper)
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::const_element_iterator > pybind11_init_const_element_iterator(py::module &m)
{
  PySharedPtrClass< smtk::mesh::const_element_iterator > instance(m, "const_element_iterator");
  instance
    .def(py::init<>())
    .def(py::init<smtk::mesh::const_element_iterator const &>())
    .def("deepcopy", (smtk::mesh::const_element_iterator & (smtk::mesh::const_element_iterator::*)(::smtk::mesh::const_element_iterator const &)) &smtk::mesh::const_element_iterator::operator=)
    ;
  return instance;
}

py::class_< smtk::mesh::HandleRange > pybind11_init_HandleRange(py::module &m)
{
  py::class_< smtk::mesh::HandleRange > instance(m, "HandleRange");
  instance
    .def(py::init<>())
    .def(py::init<smtk::mesh::HandleRange const &>())
    .def(py::init<smtk::mesh::HandleRange::domain_type const &>())
    .def(py::init<smtk::mesh::HandleRange::interval_type const &>())
    .def("__repr__", [](smtk::mesh::HandleRange& range){ std::stringstream s; s << range; return s.str(); })
    .def("begin", [](smtk::mesh::HandleRange& range){ return range.begin(); })
    .def("deepcopy", (smtk::mesh::HandleRange & (smtk::mesh::HandleRange::*)(smtk::mesh::HandleRange)) &smtk::mesh::HandleRange::operator=)
    .def("elements_begin", [](smtk::mesh::HandleRange& range){ return smtk::mesh::rangeElementsBegin(range); })
    .def("elements_end", [](smtk::mesh::HandleRange& range){ return smtk::mesh::rangeElementsEnd(range); })
    .def("end", [](smtk::mesh::HandleRange& range){ return range.end(); })
    .def("size", &smtk::mesh::HandleRange::size)
    ;
  return instance;
}

void pybind11_init_smtk_mesh_rangeElementsBegin(py::module &m)
{
  m.def("rangeElementsBegin", &smtk::mesh::rangeElementsBegin, "", py::arg("arg0"));
}

void pybind11_init_smtk_mesh_rangeElementsEnd(py::module &m)
{
  m.def("rangeElementsEnd", &smtk::mesh::rangeElementsEnd, "", py::arg("arg0"));
}

void pybind11_init_smtk_mesh_rangeElement(py::module &m)
{
  m.def("rangeElement", &smtk::mesh::rangeElement, "", py::arg("arg0"), py::arg("arg1"));
}

void pybind11_init_smtk_mesh_rangeContains(py::module &m)
{
  m.def("rangeContains", (bool (*)(smtk::mesh::HandleRange const &, smtk::mesh::Handle)) &smtk::mesh::rangeContains, "", py::arg("arg0"), py::arg("arg1"));
  m.def("rangeContains", (bool (*)(smtk::mesh::HandleRange const &, smtk::mesh::HandleRange const &)) &smtk::mesh::rangeContains, "", py::arg("super"), py::arg("sub"));
}

void pybind11_init_smtk_mesh_rangeIndex(py::module &m)
{
  m.def("rangeIndex", &smtk::mesh::rangeIndex, "", py::arg("arg0"), py::arg("arg1"));
}

void pybind11_init_smtk_mesh_rangeIntervalCount(py::module &m)
{
  m.def("rangeIntervalCount", &smtk::mesh::rangeIntervalCount, "", py::arg("arg0"));
}

void pybind11_init_smtk_mesh_rangesEqual(py::module &m)
{
  m.def("rangesEqual", &smtk::mesh::rangesEqual, "", py::arg("arg0"), py::arg("arg1"));
}

#endif
