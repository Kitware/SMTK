//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_Wrapper_h
#define pybind_smtk_resource_Wrapper_h

#include <pybind11/pybind11.h>

#include "smtk/resource/Wrapper.h"

namespace py = pybind11;

py::class_< smtk::resource::Wrapper > pybind11_init_smtk_resource_Wrapper(py::module &m)
{
  py::class_< smtk::resource::Wrapper > instance(m, "Wrapper");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::resource::Wrapper const &>())
    .def("deepcopy", (smtk::resource::Wrapper & (smtk::resource::Wrapper::*)(::smtk::resource::Wrapper const &)) &smtk::resource::Wrapper::operator=)
    .def_readwrite("id", &smtk::resource::Wrapper::id)
    .def_readwrite("link", &smtk::resource::Wrapper::link)
    .def_readwrite("resource", &smtk::resource::Wrapper::resource)
    .def_readwrite("role", &smtk::resource::Wrapper::role)
    .def_readwrite("state", &smtk::resource::Wrapper::state)
    ;
  return instance;
}

#endif
