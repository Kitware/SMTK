//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_project_Metadata_h
#define pybind_smtk_project_Metadata_h

#include <pybind11/pybind11.h>

#include "smtk/project/Metadata.h"

namespace py = pybind11;

inline py::class_< smtk::project::Metadata > pybind11_init_smtk_project_Metadata(py::module &m)
{
  py::class_< smtk::project::Metadata > instance(m, "Metadata");
  instance
    .def(py::init<::smtk::project::Metadata const &>())
    .def(py::init<::std::string const &, ::smtk::project::Project::Index, ::std::function<std::shared_ptr<smtk::project::Project> (const smtk::common::UUID &, const std::shared_ptr<smtk::common::Managers>&)>, ::std::set<std::basic_string<char>, std::less<std::basic_string<char> >, std::allocator<std::basic_string<char> > > const &, ::std::set<std::basic_string<char>, std::less<std::basic_string<char> >, std::allocator<std::basic_string<char> > > const &, ::std::string const &>())
    .def("index", &smtk::project::Metadata::index)
    .def("operations", &smtk::project::Metadata::operations)
    .def("resources", &smtk::project::Metadata::resources)
    .def("typeName", &smtk::project::Metadata::typeName)
    .def("version", &smtk::project::Metadata::version)
    .def_readonly("create", &smtk::project::Metadata::create)
    ;
  return instance;
}

#endif
