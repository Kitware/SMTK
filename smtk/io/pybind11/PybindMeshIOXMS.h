//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_mesh_MeshIOXMS_h
#define pybind_smtk_io_mesh_MeshIOXMS_h

#include <pybind11/pybind11.h>

#include "smtk/io/mesh/MeshIOXMS.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::io::mesh::MeshIOXMS > pybind11_init_smtk_io_mesh_MeshIOXMS(py::module &m, PySharedPtrClass< smtk::io::mesh::MeshIO >& parent)
{
  PySharedPtrClass< smtk::io::mesh::MeshIOXMS > instance(m, "MeshIOXMS", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::io::mesh::MeshIOXMS const &>())
    .def("deepcopy", (smtk::io::mesh::MeshIOXMS & (smtk::io::mesh::MeshIOXMS::*)(::smtk::io::mesh::MeshIOXMS const &)) &smtk::io::mesh::MeshIOXMS::operator=)
    .def("exportMesh", (bool (smtk::io::mesh::MeshIOXMS::*)(::std::ostream &, ::smtk::mesh::ResourcePtr, ::smtk::mesh::DimensionType) const) &smtk::io::mesh::MeshIOXMS::exportMesh, py::arg("stream"), py::arg("meshResource"), py::arg("dim"))
    .def("exportMesh", (bool (smtk::io::mesh::MeshIOXMS::*)(::std::string const &, ::smtk::mesh::ResourcePtr, ::smtk::mesh::DimensionType) const) &smtk::io::mesh::MeshIOXMS::exportMesh, py::arg("filePath"), py::arg("meshResource"), py::arg("dim"))
    .def("exportMesh", (bool (smtk::io::mesh::MeshIOXMS::*)(::std::string const &, ::smtk::mesh::ResourcePtr) const) &smtk::io::mesh::MeshIOXMS::exportMesh, py::arg("filePath"), py::arg("meshResource"))
    .def("exportMesh", (bool (smtk::io::mesh::MeshIOXMS::*)(::std::ostream &, ::smtk::mesh::ResourcePtr, ::smtk::model::ResourcePtr, ::std::string const &, ::smtk::mesh::DimensionType) const) &smtk::io::mesh::MeshIOXMS::exportMesh, py::arg("stream"), py::arg("meshResource"), py::arg("resource"), py::arg("modelPropertyName"), py::arg("dim"))
    .def("exportMesh", (bool (smtk::io::mesh::MeshIOXMS::*)(::std::string const &, ::smtk::mesh::ResourcePtr, ::smtk::model::ResourcePtr, ::std::string const &, ::smtk::mesh::DimensionType) const) &smtk::io::mesh::MeshIOXMS::exportMesh, py::arg("filePath"), py::arg("meshResource"), py::arg("resource"), py::arg("modelPropertyName"), py::arg("dim"))
    .def("exportMesh", (bool (smtk::io::mesh::MeshIOXMS::*)(::std::string const &, ::smtk::mesh::ResourcePtr, ::smtk::model::ResourcePtr, ::std::string const &) const) &smtk::io::mesh::MeshIOXMS::exportMesh, py::arg("filePath"), py::arg("meshResource"), py::arg("resource"), py::arg("modelPropertyName"))
    ;
  return instance;
}

#endif
