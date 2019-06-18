//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_ExtractCanonicalIndices_h
#define pybind_smtk_mesh_ExtractCanonicalIndices_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/utility/ExtractCanonicalIndices.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::utility::PreAllocatedCanonicalIndices > pybind11_init_smtk_mesh_PreAllocatedCanonicalIndices(py::module &m)
{
  PySharedPtrClass< smtk::mesh::utility::PreAllocatedCanonicalIndices > instance(m, "PreAllocatedCanonicalIndices");
  instance
    .def(py::init<::smtk::mesh::utility::PreAllocatedCanonicalIndices const &>())
    .def(py::init<::int64_t *, ::int64_t *>())
    .def("deepcopy", (smtk::mesh::utility::PreAllocatedCanonicalIndices & (smtk::mesh::utility::PreAllocatedCanonicalIndices::*)(::smtk::mesh::utility::PreAllocatedCanonicalIndices const &)) &smtk::mesh::utility::PreAllocatedCanonicalIndices::operator=)
    .def_static("determineAllocationLengths", &smtk::mesh::utility::PreAllocatedCanonicalIndices::determineAllocationLengths, py::arg("ms"), py::arg("numberOfCells"))
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::utility::CanonicalIndices > pybind11_init_smtk_mesh_CanonicalIndices(py::module &m)
{
  PySharedPtrClass< smtk::mesh::utility::CanonicalIndices > instance(m, "CanonicalIndices");
  instance
    .def(py::init<::smtk::mesh::utility::CanonicalIndices const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::mesh::utility::CanonicalIndices & (smtk::mesh::utility::CanonicalIndices::*)(::smtk::mesh::utility::CanonicalIndices const &)) &smtk::mesh::utility::CanonicalIndices::operator=)
    .def("referenceCellIndices", &smtk::mesh::utility::CanonicalIndices::referenceCellIndices)
    .def("canonicalIndices", &smtk::mesh::utility::CanonicalIndices::canonicalIndices)
    .def("extract", (void (smtk::mesh::utility::CanonicalIndices::*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::MeshSet const &)) &smtk::mesh::utility::CanonicalIndices::extract, py::arg("ms"), py::arg("reference_ms"))
    ;
  return instance;
}

void pybind11_init__extractCanonicalIndices(py::module &m)
{
  m.def("extractCanonicalIndices", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::MeshSet const &, ::smtk::mesh::utility::PreAllocatedCanonicalIndices &)) &smtk::mesh::utility::extractCanonicalIndices, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"));
}

#endif
