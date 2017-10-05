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

namespace py = pybind11;

PySharedPtrClass< smtk::resource::Resource > pybind11_init_smtk_resource_Resource(py::module &m)
{
  PySharedPtrClass< smtk::resource::Resource > instance(m, "Resource");
  instance
    .def_static("type2String", &smtk::resource::Resource::type2String, py::arg("t"))
    .def_static("string2Type", &smtk::resource::Resource::string2Type, py::arg("s"))
    .def("type", &smtk::resource::Resource::type)
    .def("index", &smtk::resource::Resource::index)
    .def("id", &smtk::resource::Resource::id)
    .def("location", &smtk::resource::Resource::location)
    .def("setId", &smtk::resource::Resource::setId)
    .def("setLocation", &smtk::resource::Resource::setLocation)
    ;
  py::enum_<smtk::resource::Resource::Type>(instance, "Type")
    .value("ATTRIBUTE", smtk::resource::Resource::Type::ATTRIBUTE)
    .value("MODEL", smtk::resource::Resource::Type::MODEL)
    .value("MESH", smtk::resource::Resource::Type::MESH)
    .value("NUMBER_OF_TYPES", smtk::resource::Resource::Type::NUMBER_OF_TYPES)
    .export_values();
  return instance;
}

#endif
