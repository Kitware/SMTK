//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_ReadMesh_h
#define pybind_smtk_io_ReadMesh_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/io/ReadMesh.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::ReadMesh > pybind11_init_smtk_io_ReadMesh(py::module &m)
{
  PySharedPtrClass< smtk::io::ReadMesh > instance(m, "ReadMesh");
  instance
    .def(py::init<>())
    .def_static("ExtensionIsSupported", &smtk::io::ReadMesh::ExtensionIsSupported)
    .def("__call__", (smtk::mesh::ResourcePtr (smtk::io::ReadMesh::*)(::std::string const &, const ::smtk::mesh::InterfacePtr&, ::smtk::io::mesh::Subset) const) &smtk::io::ReadMesh::operator())
    .def("__call__", (bool (smtk::io::ReadMesh::*)(::std::string const &, ::smtk::mesh::ResourcePtr, ::smtk::io::mesh::Subset) const) &smtk::io::ReadMesh::operator())
    ;
  return instance;
}

void pybind11_init__ZN4smtk2io13readDirichletERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7InterfaceEEE(py::module &m)
{
  m.def("readDirichlet", (smtk::mesh::ResourcePtr (*)(::std::string const &, const ::smtk::mesh::InterfacePtr&)) &smtk::io::readDirichlet, "", py::arg("filePath"), py::arg("interface"));
}

void pybind11_init__ZN4smtk2io13readDirichletERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(py::module &m)
{
  m.def("readDirichlet", (bool (*)(::std::string const &, ::smtk::mesh::ResourcePtr)) &smtk::io::readDirichlet, "", py::arg("filePath"), py::arg("resource"));
}

void pybind11_init__ZN4smtk2io10readDomainERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7InterfaceEEE(py::module &m)
{
  m.def("readDomain", (smtk::mesh::ResourcePtr (*)(::std::string const &, const ::smtk::mesh::InterfacePtr&)) &smtk::io::readDomain, "", py::arg("filePath"), py::arg("interface"));
}

void pybind11_init__ZN4smtk2io10readDomainERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(py::module &m)
{
  m.def("readDomain", (bool (*)(::std::string const &, ::smtk::mesh::ResourcePtr)) &smtk::io::readDomain, "", py::arg("filePath"), py::arg("resource"));
}

void pybind11_init__ZN4smtk2io20readEntireResourceERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7InterfaceEEE(py::module &m)
{
  m.def("readEntireResource", (smtk::mesh::ResourcePtr (*)(::std::string const &, const ::smtk::mesh::InterfacePtr&)) &smtk::io::readEntireResource, "", py::arg("filePath"), py::arg("interface"));
}

void pybind11_init__ZN4smtk2io20readEntireResourceERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(py::module &m)
{
  m.def("readEntireResource", (bool (*)(::std::string const &, ::smtk::mesh::ResourcePtr)) &smtk::io::readEntireResource, "", py::arg("filePath"), py::arg("resource"));
}

void pybind11_init__ZN4smtk2io8readMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7InterfaceEEENS0_4mesh6SubsetE(py::module &m)
{
  m.def("readMesh", (smtk::mesh::ResourcePtr (*)(::std::string const &, const ::smtk::mesh::InterfacePtr&, ::smtk::io::mesh::Subset)) &smtk::io::readMesh, "", py::arg("filePath"), py::arg("interface"), py::arg("subset") = ::smtk::io::mesh::Subset::EntireResource);
}

void pybind11_init__ZN4smtk2io8readMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEENS0_4mesh6SubsetE(py::module &m)
{
  m.def("readMesh", (bool (*)(::std::string const &, ::smtk::mesh::ResourcePtr, ::smtk::io::mesh::Subset)) &smtk::io::readMesh, "", py::arg("filePath"), py::arg("resource"), py::arg("subset") = ::smtk::io::mesh::Subset::EntireResource);
}

void pybind11_init__ZN4smtk2io11readNeumannERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7InterfaceEEE(py::module &m)
{
  m.def("readNeumann", (smtk::mesh::ResourcePtr (*)(::std::string const &, const ::smtk::mesh::InterfacePtr&)) &smtk::io::readNeumann, "", py::arg("filePath"), py::arg("interface"));
}

void pybind11_init__ZN4smtk2io11readNeumannERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(py::module &m)
{
  m.def("readNeumann", (bool (*)(::std::string const &, ::smtk::mesh::ResourcePtr)) &smtk::io::readNeumann, "", py::arg("filePath"), py::arg("resource"));
}

#endif
