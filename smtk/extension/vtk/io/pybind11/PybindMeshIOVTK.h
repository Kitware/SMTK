//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_vtk_io_mesh_MeshIOVTK_h
#define pybind_smtk_extension_vtk_io_mesh_MeshIOVTK_h

#include <pybind11/pybind11.h>

#include "smtk/extension/vtk/io/mesh/MeshIOVTK.h"

#include "smtk/io/mesh/MeshIO.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::extension::vtk::io::mesh::MeshIOVTK, smtk::io::mesh::MeshIO > pybind11_init_smtk_extension_vtk_io_mesh_MeshIOVTK(py::module &m)
{
  PySharedPtrClass< smtk::extension::vtk::io::mesh::MeshIOVTK, smtk::io::mesh::MeshIO > instance(m, "MeshIOVTK");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::extension::vtk::io::mesh::MeshIOVTK const &>())
    .def("deepcopy", (smtk::extension::vtk::io::mesh::MeshIOVTK & (smtk::extension::vtk::io::mesh::MeshIOVTK::*)(::smtk::extension::vtk::io::mesh::MeshIOVTK const &)) &smtk::extension::vtk::io::mesh::MeshIOVTK::operator=)
    .def("importMesh", (smtk::mesh::ResourcePtr (smtk::extension::vtk::io::mesh::MeshIOVTK::*)(::std::string const &, const ::smtk::mesh::InterfacePtr &, ::std::string const &) const) &smtk::extension::vtk::io::mesh::MeshIOVTK::importMesh, py::arg("filePath"), py::arg("interface"), py::arg("domainPropertyName"))
    .def("importMesh", (bool (smtk::extension::vtk::io::mesh::MeshIOVTK::*)(::std::string const &, ::smtk::mesh::ResourcePtr, ::std::string const &) const) &smtk::extension::vtk::io::mesh::MeshIOVTK::importMesh, py::arg("filePath"), py::arg("resource"), py::arg("domainPropertyName"))
    .def("exportMesh", &smtk::extension::vtk::io::mesh::MeshIOVTK::exportMesh, py::arg("filePath"), py::arg("resource"))
    ;
  return instance;
}

#endif
