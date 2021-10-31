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

#include "smtk/resource/pybind11/PyResource.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::resource::Resource, smtk::resource::PyResource, smtk::resource::PersistentObject > pybind11_init_smtk_resource_Resource(py::module &m)
{
  PySharedPtrClass< smtk::resource::Resource, smtk::resource::PyResource, smtk::resource::PersistentObject > instance(m, "Resource");
  instance
    .def(py::init<>())
    .def_static("create", &smtk::resource::PyResource::create)
    .def("index", &smtk::resource::Resource::index)
    .def("id", &smtk::resource::Resource::id)
    .def("location", &smtk::resource::Resource::location)
    .def("name", (std::string (smtk::resource::Resource::*)() const) &smtk::resource::Resource::name)
    .def("setName", &smtk::resource::Resource::setName)
   .def("setId", &smtk::resource::Resource::setId)
    .def("setLocation", &smtk::resource::Resource::setLocation)
    .def("filter", (smtk::resource::ComponentSet (smtk::resource::Resource::*)(const std::string&) const) &smtk::resource::Resource::filter)
    .def("find", (smtk::resource::Component::Ptr (smtk::resource::Resource::*)(const smtk::common::UUID&) const) &smtk::resource::Resource::find)
    .def("manager", &smtk::resource::Resource::manager)

    // SMTK_DEPRECATED_IN_21_09("Replaced by Resource.filter")
    .def("find", (smtk::resource::ComponentSet (smtk::resource::Resource::*)(const std::string&) const) &smtk::resource::Resource::filter)
    ;
  return instance;
}

#endif
