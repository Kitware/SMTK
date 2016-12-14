//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_ContainsFunctors_h
#define pybind_smtk_mesh_ContainsFunctors_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/ContainsFunctors.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::ContainsFunctor > pybind11_init_smtk_mesh_ContainsFunctor(py::module &m)
{
  PySharedPtrClass< smtk::mesh::ContainsFunctor > instance(m, "ContainsFunctor");
  instance
    .def("__call__", (bool (smtk::mesh::ContainsFunctor::*)(::smtk::mesh::HandleRange const &, ::smtk::mesh::Handle const *, ::size_t const) const) &smtk::mesh::ContainsFunctor::operator())
    .def("deepcopy", (smtk::mesh::ContainsFunctor & (smtk::mesh::ContainsFunctor::*)(::smtk::mesh::ContainsFunctor const &)) &smtk::mesh::ContainsFunctor::operator=)
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::PartiallyContainedFunctor > pybind11_init_smtk_mesh_PartiallyContainedFunctor(py::module &m, PySharedPtrClass< smtk::mesh::ContainsFunctor >& parent)
{
  PySharedPtrClass< smtk::mesh::PartiallyContainedFunctor > instance(m, "PartiallyContainedFunctor", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::mesh::PartiallyContainedFunctor const &>())
    .def("__call__", (bool (smtk::mesh::PartiallyContainedFunctor::*)(::smtk::mesh::HandleRange const &, ::smtk::mesh::Handle const *, ::size_t const) const) &smtk::mesh::PartiallyContainedFunctor::operator())
    .def("deepcopy", (smtk::mesh::PartiallyContainedFunctor & (smtk::mesh::PartiallyContainedFunctor::*)(::smtk::mesh::PartiallyContainedFunctor const &)) &smtk::mesh::PartiallyContainedFunctor::operator=)
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::FullyContainedFunctor > pybind11_init_smtk_mesh_FullyContainedFunctor(py::module &m, PySharedPtrClass< smtk::mesh::ContainsFunctor >& parent)
{
  PySharedPtrClass< smtk::mesh::FullyContainedFunctor > instance(m, "FullyContainedFunctor", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::mesh::FullyContainedFunctor const &>())
    .def("__call__", (bool (smtk::mesh::FullyContainedFunctor::*)(::smtk::mesh::HandleRange const &, ::smtk::mesh::Handle const *, ::size_t const) const) &smtk::mesh::FullyContainedFunctor::operator())
    .def("deepcopy", (smtk::mesh::FullyContainedFunctor & (smtk::mesh::FullyContainedFunctor::*)(::smtk::mesh::FullyContainedFunctor const &)) &smtk::mesh::FullyContainedFunctor::operator=)
    ;
  return instance;
}

#endif
