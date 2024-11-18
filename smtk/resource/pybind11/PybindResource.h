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
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/resource/Component.h"
#include "smtk/resource/Lock.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/PersistentObject.h"

#include "smtk/resource/pybind11/PyResource.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::resource::Resource, smtk::resource::PyResource, smtk::resource::PersistentObject > pybind11_init_smtk_resource_Resource(py::module &m)
{
  PySharedPtrClass< smtk::resource::Resource, smtk::resource::PyResource, smtk::resource::PersistentObject > instance(m, "Resource");
  instance
    .def(py::init<>())
    .def_static("create", &smtk::resource::PyResource::create)
    .def("clean", &smtk::resource::Resource::clean)
    .def("clone", &smtk::resource::Resource::clone)
    .def("copyInitialize", &smtk::resource::Resource::copyInitialize, py::arg("source"), py::arg("copyOptions"))
    .def("copyFinalize", &smtk::resource::Resource::copyFinalize, py::arg("source"), py::arg("copyOptions"))
    .def("copyProperties", &smtk::resource::Resource::copyProperties, py::arg("source"), py::arg("copyOptions"))
    .def("copyUnitSystem", &smtk::resource::Resource::copyUnitSystem, py::arg("source"), py::arg("copyOptions"))
    .def("filter", &smtk::resource::Resource::filter, py::arg("queryString"))
    .def("find", (smtk::resource::Component::Ptr (smtk::resource::Resource::*)(const smtk::common::UUID&) const) &smtk::resource::Resource::find)
    .def("id", &smtk::resource::Resource::id)
    .def("index", &smtk::resource::Resource::index)
    .def("isMarkedForRemoval", &smtk::resource::Resource::isMarkedForRemoval)
    .def("isNameSet", &smtk::resource::Resource::isNameSet)
    .def("isOfType", (bool (smtk::resource::Resource::*)(::smtk::resource::Resource::Index const &) const) &smtk::resource::Resource::isOfType, py::arg("index"))
    .def("isOfType", (bool (smtk::resource::Resource::*)(::std::string const &) const) &smtk::resource::Resource::isOfType, py::arg("typeName"))
    .def("links", (smtk::resource::Resource::Links & (smtk::resource::Resource::*)()) &smtk::resource::Resource::links)
    .def("links", (smtk::resource::Resource::Links const & (smtk::resource::Resource::*)() const) &smtk::resource::Resource::links)
    .def("location", &smtk::resource::Resource::location)
    .def("lock", &smtk::resource::Resource::lock, py::arg("arg0"))
    .def("locked", &smtk::resource::Resource::locked)
    .def("manager", &smtk::resource::Resource::manager)
    .def("name", &smtk::resource::Resource::name)
    .def("numberOfGenerationsFromBase", &smtk::resource::Resource::numberOfGenerationsFromBase, py::arg("typeName"))
    .def("stringProperties", [](smtk::resource::Resource& resource)
      {
        smtk::resource::PropertiesOfType<std::string> props = resource.properties().get<std::string>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("doubleProperties", [](smtk::resource::Resource& resource)
      {
        smtk::resource::PropertiesOfType<double> props = resource.properties().get<double>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("intProperties", [](smtk::resource::Resource& resource)
      {
        smtk::resource::PropertiesOfType<int> props = resource.properties().get<int>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("boolProperties", [](smtk::resource::Resource& resource)
      {
        smtk::resource::PropertiesOfType<bool> props = resource.properties().get<bool>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("longProperties", [](smtk::resource::Resource& resource)
      {
        smtk::resource::PropertiesOfType<long> props = resource.properties().get<long>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("stringVectorProperties", [](smtk::resource::Resource& resource)
      {
        smtk::resource::PropertiesOfType<std::vector<std::string>> props = resource.properties().get<std::vector<std::string>>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("doubleVectorProperties", [](smtk::resource::Resource& resource)
      {
        smtk::resource::PropertiesOfType<std::vector<double>> props = resource.properties().get<std::vector<double>>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("intVectorProperties", [](smtk::resource::Resource& resource)
      {
        smtk::resource::PropertiesOfType<std::vector<int>> props = resource.properties().get<std::vector<int>>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("boolVectorProperties", [](smtk::resource::Resource& resource)
      {
        smtk::resource::PropertiesOfType<std::vector<bool>> props = resource.properties().get<std::vector<bool>>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("longVectorProperties", [](smtk::resource::Resource& resource)
      {
        smtk::resource::PropertiesOfType<std::vector<long>> props = resource.properties().get<std::vector<long>>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("stringTokenProperties", [](smtk::resource::Resource& resource)
      {
        smtk::resource::PropertiesOfType<smtk::string::Token> props = resource.properties().get<smtk::string::Token>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("stringTokenSetProperties", [](smtk::resource::Resource& resource)
      {
        smtk::resource::PropertiesOfType<std::set<smtk::string::Token>> props =
          resource.properties().get<std::set<smtk::string::Token>>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("coordinateFrameProperties", [](smtk::resource::Resource& resource)
      {
        smtk::resource::PropertiesOfType<smtk::resource::properties::CoordinateFrame> props =
          resource.properties().get<smtk::resource::properties::CoordinateFrame>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("namedCoordinateFrameProperties", [](smtk::resource::Resource& resource)
      {
        smtk::resource::PropertiesOfType<std::map<std::string, smtk::resource::properties::CoordinateFrame>> props =
          resource.properties().get<std::map<std::string, smtk::resource::properties::CoordinateFrame>>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("queries", (smtk::resource::Resource::Queries const & (smtk::resource::Resource::*)() const) &smtk::resource::Resource::queries)
    .def("queries", (smtk::resource::Resource::Queries & (smtk::resource::Resource::*)()) &smtk::resource::Resource::queries)
    .def("queryOperation", &smtk::resource::Resource::queryOperation, py::arg("arg0"))
    .def("setClean", &smtk::resource::Resource::setClean, py::arg("state") = true)
    .def("setId", &smtk::resource::Resource::setId, py::arg("id"))
    .def("setLocation", &smtk::resource::Resource::setLocation, py::arg("location"))
    .def("setManager", &smtk::resource::Resource::setManager, py::arg("manager"))
    .def("setMarkedForRemoval", &smtk::resource::Resource::setMarkedForRemoval, py::arg("val"))
    .def("setName", &smtk::resource::Resource::setName, py::arg("name"))
    .def("typeName", &smtk::resource::Resource::typeName)
    .def("visit", &smtk::resource::Resource::visit, py::arg("v"))
    .def_static("visuallyLinkedRole", &smtk::resource::Resource::visuallyLinkedRole)
    .def_readonly_static("VisuallyLinkedRole", &smtk::resource::Resource::VisuallyLinkedRole)
    .def_readonly_static("type_index", &smtk::resource::Resource::type_index)
    .def_readonly_static("type_name", &smtk::resource::Resource::type_name)
    ;
  return instance;
}

#endif
