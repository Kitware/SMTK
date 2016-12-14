//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_vtk_io_MeshIOVTK_h
#define pybind_smtk_extension_vtk_io_MeshIOVTK_h

#include <pybind11/pybind11.h>

#include "smtk/extension/vtk/io/MeshIOVTK.h"

#include "smtk/io/mesh/MeshIO.h"

namespace py = pybind11;

py::class_< smtk::extension::vtk::io::MeshIOVTK, smtk::io::mesh::MeshIO > pybind11_init_smtk_extension_vtk_io_MeshIOVTK(py::module &m)
{
  py::class_< smtk::extension::vtk::io::MeshIOVTK, smtk::io::mesh::MeshIO > instance(m, "MeshIOVTK");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::extension::vtk::io::MeshIOVTK const &>())
    .def("deepcopy", (smtk::extension::vtk::io::MeshIOVTK & (smtk::extension::vtk::io::MeshIOVTK::*)(::smtk::extension::vtk::io::MeshIOVTK const &)) &smtk::extension::vtk::io::MeshIOVTK::operator=)
    .def("importMesh", (smtk::mesh::CollectionPtr (smtk::extension::vtk::io::MeshIOVTK::*)(::std::string const &, ::smtk::mesh::ManagerPtr &, ::std::string const &) const) &smtk::extension::vtk::io::MeshIOVTK::importMesh, py::arg("filePath"), py::arg("manager"), py::arg("domainPropertyName"))
    .def("importMesh", (bool (smtk::extension::vtk::io::MeshIOVTK::*)(::std::string const &, ::smtk::mesh::CollectionPtr, ::std::string const &) const) &smtk::extension::vtk::io::MeshIOVTK::importMesh, py::arg("filePath"), py::arg("collection"), py::arg("domainPropertyName"))
    .def("exportMesh", &smtk::extension::vtk::io::MeshIOVTK::exportMesh, py::arg("filePath"), py::arg("collection"))
    ;
  return instance;
}

#endif
