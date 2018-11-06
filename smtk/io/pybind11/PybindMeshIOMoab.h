//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_mesh_MeshIOMoab_h
#define pybind_smtk_io_mesh_MeshIOMoab_h

#include <pybind11/pybind11.h>

#include "smtk/io/mesh/MeshIOMoab.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::mesh::MeshIOMoab > pybind11_init_smtk_io_mesh_MeshIOMoab(py::module &m, PySharedPtrClass< smtk::io::mesh::MeshIO >& parent)
{
  PySharedPtrClass< smtk::io::mesh::MeshIOMoab > instance(m, "MeshIOMoab", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::io::mesh::MeshIOMoab const &>())
    .def("deepcopy", (smtk::io::mesh::MeshIOMoab & (smtk::io::mesh::MeshIOMoab::*)(::smtk::io::mesh::MeshIOMoab const &)) &smtk::io::mesh::MeshIOMoab::operator=)
    .def("importMesh", (smtk::mesh::ResourcePtr (smtk::io::mesh::MeshIOMoab::*)(::std::string const &, const ::smtk::mesh::InterfacePtr &, ::std::string const &) const) &smtk::io::mesh::MeshIOMoab::importMesh, py::arg("filePath"), py::arg("interface"), py::arg("arg2"))
    .def("importMesh", (bool (smtk::io::mesh::MeshIOMoab::*)(::std::string const &, ::smtk::mesh::ResourcePtr, ::std::string const &) const) &smtk::io::mesh::MeshIOMoab::importMesh, py::arg("filePath"), py::arg("resource"), py::arg("arg2"))
    .def("exportMesh", &smtk::io::mesh::MeshIOMoab::exportMesh, py::arg("filePath"), py::arg("resource"))
    .def("read", (smtk::mesh::ResourcePtr (smtk::io::mesh::MeshIOMoab::*)(::std::string const &, const ::smtk::mesh::InterfacePtr &, ::smtk::io::mesh::Subset) const) &smtk::io::mesh::MeshIOMoab::read, py::arg("filePath"), py::arg("interface"), py::arg("s"))
    .def("read", (bool (smtk::io::mesh::MeshIOMoab::*)(::std::string const &, ::smtk::mesh::ResourcePtr, ::smtk::io::mesh::Subset) const) &smtk::io::mesh::MeshIOMoab::read, py::arg("filePath"), py::arg("resource"), py::arg("s"))
    .def("write", (bool (smtk::io::mesh::MeshIOMoab::*)(::std::string const &, ::smtk::mesh::ResourcePtr, ::smtk::io::mesh::Subset) const) &smtk::io::mesh::MeshIOMoab::write, py::arg("filePath"), py::arg("resource"), py::arg("s"))
    .def("write", (bool (smtk::io::mesh::MeshIOMoab::*)(::smtk::mesh::ResourcePtr, ::smtk::io::mesh::Subset) const) &smtk::io::mesh::MeshIOMoab::write, py::arg("resource"), py::arg("s"))
    ;
  return instance;
}

#endif
