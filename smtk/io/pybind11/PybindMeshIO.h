//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_mesh_MeshIO_h
#define pybind_smtk_io_mesh_MeshIO_h

#include <pybind11/pybind11.h>

#include "smtk/io/mesh/MeshIO.h"

namespace py = pybind11;

inline void pybind11_init_smtk_io_mesh_Subset(py::module &m)
{
  py::enum_<smtk::io::mesh::Subset>(m, "Subset")
    .value("EntireResource", smtk::io::mesh::Subset::EntireResource)
    .value("OnlyDomain", smtk::io::mesh::Subset::OnlyDomain)
    .value("OnlyDirichlet", smtk::io::mesh::Subset::OnlyDirichlet)
    .value("OnlyNeumann", smtk::io::mesh::Subset::OnlyNeumann)
    .export_values();
}

inline PySharedPtrClass< smtk::io::mesh::MeshIO > pybind11_init_smtk_io_mesh_MeshIO(py::module &m)
{
  PySharedPtrClass< smtk::io::mesh::MeshIO > instance(m, "MeshIO");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::io::mesh::MeshIO const &>())
    .def("deepcopy", (smtk::io::mesh::MeshIO & (smtk::io::mesh::MeshIO::*)(::smtk::io::mesh::MeshIO const &)) &smtk::io::mesh::MeshIO::operator=)
    .def("importMesh", (smtk::mesh::ResourcePtr (smtk::io::mesh::MeshIO::*)(::std::string const &, const ::smtk::mesh::InterfacePtr &, ::std::string const &) const) &smtk::io::mesh::MeshIO::importMesh, py::arg("arg0"), py::arg("arg1"), py::arg("arg2"))
    .def("importMesh", (bool (smtk::io::mesh::MeshIO::*)(::std::string const &, ::smtk::mesh::ResourcePtr, ::std::string const &) const) &smtk::io::mesh::MeshIO::importMesh, py::arg("arg0"), py::arg("arg1"), py::arg("arg2"))
    .def("exportMesh", (bool (smtk::io::mesh::MeshIO::*)(::std::string const &, ::smtk::mesh::ResourcePtr) const) &smtk::io::mesh::MeshIO::exportMesh, py::arg("arg0"), py::arg("arg1"))
    .def("exportMesh", (bool (smtk::io::mesh::MeshIO::*)(::std::string const &, ::smtk::mesh::ResourcePtr, ::smtk::model::ResourcePtr, ::std::string const &) const) &smtk::io::mesh::MeshIO::exportMesh, py::arg("arg0"), py::arg("arg1"), py::arg("arg2"), py::arg("arg3"))
    .def("read", (smtk::mesh::ResourcePtr (smtk::io::mesh::MeshIO::*)(::std::string const &, const ::smtk::mesh::InterfacePtr &, ::smtk::io::mesh::Subset) const) &smtk::io::mesh::MeshIO::read, py::arg("arg0"), py::arg("arg1"), py::arg("arg2"))
    .def("read", (bool (smtk::io::mesh::MeshIO::*)(::std::string const &, ::smtk::mesh::ResourcePtr, ::smtk::io::mesh::Subset) const) &smtk::io::mesh::MeshIO::read, py::arg("arg0"), py::arg("arg1"), py::arg("arg2"))
    .def("write", (bool (smtk::io::mesh::MeshIO::*)(::std::string const &, ::smtk::mesh::ResourcePtr, ::smtk::io::mesh::Subset) const) &smtk::io::mesh::MeshIO::write, py::arg("arg0"), py::arg("arg1"), py::arg("arg2"))
    .def("write", (bool (smtk::io::mesh::MeshIO::*)(::smtk::mesh::ResourcePtr, ::smtk::io::mesh::Subset) const) &smtk::io::mesh::MeshIO::write, py::arg("arg0"), py::arg("arg1"))
    .def("FileFormats", &smtk::io::mesh::MeshIO::FileFormats)
    ;
  return instance;
}

#endif
