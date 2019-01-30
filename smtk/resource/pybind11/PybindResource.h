//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_Resource_h
#define pybind_smtk_resource_Resource_h

#include <pybind11/pybind11.h>

#include "smtk/resource/Resource.h"
#include "smtk/resource/PersistentObject.h"

namespace py = pybind11;

PySharedPtrClass< smtk::resource::Resource, smtk::resource::PersistentObject > pybind11_init_smtk_resource_Resource(py::module &m)
{
  PySharedPtrClass< smtk::resource::Resource, smtk::resource::PersistentObject > instance(m, "Resource");
  instance
    .def("index", &smtk::resource::Resource::index)
    .def("id", &smtk::resource::Resource::id)
    .def("location", &smtk::resource::Resource::location)
    .def("name", (std::string (smtk::resource::Resource::*)() const) &smtk::resource::Resource::name)
    .def("setName", (bool (smtk::resource::Resource::*)(::std::string const &)) &smtk::resource::Resource::name, py::arg("name"))
   .def("setId", &smtk::resource::Resource::setId)
    .def("setLocation", &smtk::resource::Resource::setLocation)
    .def("find", (smtk::resource::ComponentSet (smtk::resource::Resource::*)(const std::string&) const) &smtk::resource::Resource::find)
    .def("manager", &smtk::resource::Resource::manager)
    ;
  return instance;
}

#endif
