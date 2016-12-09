//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_ExportMesh_h
#define pybind_smtk_io_ExportMesh_h

#include <pybind11/pybind11.h>

#include "smtk/io/ExportMesh.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::ExportMesh > pybind11_init_smtk_io_ExportMesh(py::module &m)
{
  PySharedPtrClass< smtk::io::ExportMesh > instance(m, "ExportMesh");
  instance
    .def(py::init<>())
    .def("__call__", (bool (smtk::io::ExportMesh::*)(::std::string const &, ::smtk::mesh::CollectionPtr) const) &smtk::io::ExportMesh::operator())
    .def("__call__", (bool (smtk::io::ExportMesh::*)(::std::string const &, ::smtk::mesh::CollectionPtr, ::smtk::model::ManagerPtr, ::std::string const &) const) &smtk::io::ExportMesh::operator())
    // .def_static("SupportedIOTypes", &smtk::io::ExportMesh::SupportedIOTypes)
    ;
  return instance;
}

void pybind11_init__ZN4smtk2io10exportMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEE(py::module &m)
{
  m.def("exportMesh", (bool (*)(::std::string const &, ::smtk::mesh::CollectionPtr)) &smtk::io::exportMesh, "", py::arg("filePath"), py::arg("collection"));
}

void pybind11_init__ZN4smtk2io10exportMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEENSA_INS_5model7ManagerEEES9_(py::module &m)
{
  m.def("exportMesh", (bool (*)(::std::string const &, ::smtk::mesh::CollectionPtr, ::smtk::model::ManagerPtr, ::std::string const &)) &smtk::io::exportMesh, "", py::arg("filePath"), py::arg("collection"), py::arg("manager"), py::arg("modelPropertyName"));
}

#endif
