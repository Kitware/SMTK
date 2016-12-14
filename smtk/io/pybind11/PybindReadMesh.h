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

#include "smtk/io/ReadMesh.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::ReadMesh > pybind11_init_smtk_io_ReadMesh(py::module &m)
{
  PySharedPtrClass< smtk::io::ReadMesh > instance(m, "ReadMesh");
  instance
    .def(py::init<>())
    .def("__call__", (smtk::mesh::CollectionPtr (smtk::io::ReadMesh::*)(::std::string const &, ::smtk::mesh::ManagerPtr, ::smtk::io::mesh::Subset) const) &smtk::io::ReadMesh::operator())
    .def("__call__", (bool (smtk::io::ReadMesh::*)(::std::string const &, ::smtk::mesh::CollectionPtr, ::smtk::io::mesh::Subset) const) &smtk::io::ReadMesh::operator())
    ;
  return instance;
}

void pybind11_init__ZN4smtk2io13readDirichletERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7ManagerEEE(py::module &m)
{
  m.def("readDirichlet", (smtk::mesh::CollectionPtr (*)(::std::string const &, ::smtk::mesh::ManagerPtr)) &smtk::io::readDirichlet, "", py::arg("filePath"), py::arg("manager"));
}

void pybind11_init__ZN4smtk2io13readDirichletERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEE(py::module &m)
{
  m.def("readDirichlet", (bool (*)(::std::string const &, ::smtk::mesh::CollectionPtr)) &smtk::io::readDirichlet, "", py::arg("filePath"), py::arg("collection"));
}

void pybind11_init__ZN4smtk2io10readDomainERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7ManagerEEE(py::module &m)
{
  m.def("readDomain", (smtk::mesh::CollectionPtr (*)(::std::string const &, ::smtk::mesh::ManagerPtr)) &smtk::io::readDomain, "", py::arg("filePath"), py::arg("manager"));
}

void pybind11_init__ZN4smtk2io10readDomainERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEE(py::module &m)
{
  m.def("readDomain", (bool (*)(::std::string const &, ::smtk::mesh::CollectionPtr)) &smtk::io::readDomain, "", py::arg("filePath"), py::arg("collection"));
}

void pybind11_init__ZN4smtk2io20readEntireCollectionERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7ManagerEEE(py::module &m)
{
  m.def("readEntireCollection", (smtk::mesh::CollectionPtr (*)(::std::string const &, ::smtk::mesh::ManagerPtr)) &smtk::io::readEntireCollection, "", py::arg("filePath"), py::arg("manager"));
}

void pybind11_init__ZN4smtk2io20readEntireCollectionERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEE(py::module &m)
{
  m.def("readEntireCollection", (bool (*)(::std::string const &, ::smtk::mesh::CollectionPtr)) &smtk::io::readEntireCollection, "", py::arg("filePath"), py::arg("collection"));
}

void pybind11_init__ZN4smtk2io8readMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7ManagerEEENS0_4mesh6SubsetE(py::module &m)
{
  m.def("readMesh", (smtk::mesh::CollectionPtr (*)(::std::string const &, ::smtk::mesh::ManagerPtr, ::smtk::io::mesh::Subset)) &smtk::io::readMesh, "", py::arg("filePath"), py::arg("manager"), py::arg("subset") = ::smtk::io::mesh::Subset::EntireCollection);
}

void pybind11_init__ZN4smtk2io8readMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEENS0_4mesh6SubsetE(py::module &m)
{
  m.def("readMesh", (bool (*)(::std::string const &, ::smtk::mesh::CollectionPtr, ::smtk::io::mesh::Subset)) &smtk::io::readMesh, "", py::arg("filePath"), py::arg("collection"), py::arg("subset") = ::smtk::io::mesh::Subset::EntireCollection);
}

void pybind11_init__ZN4smtk2io11readNeumannERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7ManagerEEE(py::module &m)
{
  m.def("readNeumann", (smtk::mesh::CollectionPtr (*)(::std::string const &, ::smtk::mesh::ManagerPtr)) &smtk::io::readNeumann, "", py::arg("filePath"), py::arg("manager"));
}

void pybind11_init__ZN4smtk2io11readNeumannERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEE(py::module &m)
{
  m.def("readNeumann", (bool (*)(::std::string const &, ::smtk::mesh::CollectionPtr)) &smtk::io::readNeumann, "", py::arg("filePath"), py::arg("collection"));
}

#endif
