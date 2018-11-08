//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_WriteMesh_h
#define pybind_smtk_io_WriteMesh_h

#include <pybind11/pybind11.h>

#include "smtk/io/WriteMesh.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::WriteMesh > pybind11_init_smtk_io_WriteMesh(py::module &m)
{
  PySharedPtrClass< smtk::io::WriteMesh > instance(m, "WriteMesh");
  instance
    .def(py::init<>())
    .def("__call__", (bool (smtk::io::WriteMesh::*)(::std::string const &, ::smtk::mesh::ResourcePtr, ::smtk::io::mesh::Subset) const) &smtk::io::WriteMesh::operator())
    .def("__call__", (bool (smtk::io::WriteMesh::*)(::smtk::mesh::ResourcePtr, ::smtk::io::mesh::Subset) const) &smtk::io::WriteMesh::operator())
    ;
  return instance;
}

void pybind11_init__ZN4smtk2io14writeDirichletERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(py::module &m)
{
  m.def("writeDirichlet", (bool (*)(::std::string const &, ::smtk::mesh::ResourcePtr)) &smtk::io::writeDirichlet, "", py::arg("filePath"), py::arg("resource"));
}

void pybind11_init__ZN4smtk2io14writeDirichletENSt3__110shared_ptrINS_4mesh10ResourceEEE(py::module &m)
{
  m.def("writeDirichlet", (bool (*)(::smtk::mesh::ResourcePtr)) &smtk::io::writeDirichlet, "", py::arg("resource"));
}

void pybind11_init__ZN4smtk2io11writeDomainERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(py::module &m)
{
  m.def("writeDomain", (bool (*)(::std::string const &, ::smtk::mesh::ResourcePtr)) &smtk::io::writeDomain, "", py::arg("filePath"), py::arg("resource"));
}

void pybind11_init__ZN4smtk2io11writeDomainENSt3__110shared_ptrINS_4mesh10ResourceEEE(py::module &m)
{
  m.def("writeDomain", (bool (*)(::smtk::mesh::ResourcePtr)) &smtk::io::writeDomain, "", py::arg("resource"));
}

void pybind11_init__ZN4smtk2io21writeEntireResourceERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(py::module &m)
{
  m.def("writeEntireResource", (bool (*)(::std::string const &, ::smtk::mesh::ResourcePtr)) &smtk::io::writeEntireResource, "", py::arg("filePath"), py::arg("resource"));
}

void pybind11_init__ZN4smtk2io21writeEntireResourceENSt3__110shared_ptrINS_4mesh10ResourceEEE(py::module &m)
{
  m.def("writeEntireResource", (bool (*)(::smtk::mesh::ResourcePtr)) &smtk::io::writeEntireResource, "", py::arg("resource"));
}

void pybind11_init__ZN4smtk2io9writeMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEENS0_4mesh6SubsetE(py::module &m)
{
  m.def("writeMesh", (bool (*)(::std::string const &, ::smtk::mesh::ResourcePtr, ::smtk::io::mesh::Subset)) &smtk::io::writeMesh, "", py::arg("filePath"), py::arg("resource"), py::arg("subset") = ::smtk::io::mesh::Subset::EntireResource);
}

void pybind11_init__ZN4smtk2io9writeMeshENSt3__110shared_ptrINS_4mesh10ResourceEEENS0_4mesh6SubsetE(py::module &m)
{
  m.def("writeMesh", (bool (*)(::smtk::mesh::ResourcePtr, ::smtk::io::mesh::Subset)) &smtk::io::writeMesh, "", py::arg("resource"), py::arg("subset") = ::smtk::io::mesh::Subset::EntireResource);
}

void pybind11_init__ZN4smtk2io12writeNeumannERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(py::module &m)
{
  m.def("writeNeumann", (bool (*)(::std::string const &, ::smtk::mesh::ResourcePtr)) &smtk::io::writeNeumann, "", py::arg("filePath"), py::arg("resource"));
}

void pybind11_init__ZN4smtk2io12writeNeumannENSt3__110shared_ptrINS_4mesh10ResourceEEE(py::module &m)
{
  m.def("writeNeumann", (bool (*)(::smtk::mesh::ResourcePtr)) &smtk::io::writeNeumann, "", py::arg("resource"));
}

#endif
