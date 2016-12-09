//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_mesh_Format_h
#define pybind_smtk_io_mesh_Format_h

#include <pybind11/pybind11.h>

#include "smtk/io/mesh/Format.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::mesh::Format > pybind11_init_smtk_io_mesh_Format(py::module &m)
{
  PySharedPtrClass< smtk::io::mesh::Format > instance(m, "Format");
  instance
    .def(py::init<>())
    .def(py::init<::std::string const &, ::smtk::io::mesh::Format::IOFlags>())
    .def(py::init<::std::string const &, ::std::vector<std::basic_string<char>, std::allocator<std::basic_string<char> > > const &, ::smtk::io::mesh::Format::IOFlags>())
    .def(py::init<::std::string const &, ::std::string const &, ::smtk::io::mesh::Format::IOFlags>())
    .def(py::init<::smtk::io::mesh::Format const &>())
    .def("deepcopy", (smtk::io::mesh::Format & (smtk::io::mesh::Format::*)(::smtk::io::mesh::Format const &)) &smtk::io::mesh::Format::operator=)
    .def("CanImport", &smtk::io::mesh::Format::CanImport)
    .def("CanExport", &smtk::io::mesh::Format::CanExport)
    .def("CanRead", &smtk::io::mesh::Format::CanRead)
    .def("CanWrite", &smtk::io::mesh::Format::CanWrite)
    .def_readwrite("Name", &smtk::io::mesh::Format::Name)
    .def_readwrite("Extensions", &smtk::io::mesh::Format::Extensions)
    .def_readwrite("Flags", &smtk::io::mesh::Format::Flags)
    ;
  return instance;
}

#endif
