//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_Component_h
#define pybind_smtk_resource_Component_h

#include <pybind11/pybind11.h>

#include "smtk/resource/Component.h"
#include "smtk/resource/ComponentLinks.h"
#include "smtk/resource/PersistentObject.h"

#include "smtk/resource/Resource.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

#include "smtk/resource/pybind11/PyComponent.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::resource::Component, smtk::resource::PyComponent, smtk::resource::PersistentObject > pybind11_init_smtk_resource_Component(py::module &m)
{
  PySharedPtrClass< smtk::resource::Component, smtk::resource::PyComponent, smtk::resource::PersistentObject > instance(m, "Component");
  instance
    .def(py::init<>())
    .def("links", (smtk::resource::Component::Links& (smtk::resource::Component::*)()) &smtk::resource::Component::links, py::return_value_policy::reference_internal)
    .def("deepcopy", (smtk::resource::Component & (smtk::resource::Component::*)(::smtk::resource::Component const &)) &smtk::resource::Component::operator=)
    .def("resource", &smtk::resource::Component::resource)
    .def("stringProperties", [](smtk::resource::Component& component)
      {
        smtk::resource::PropertiesOfType<std::string> props = component.properties().get<std::string>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("doubleProperties", [](smtk::resource::Component& component)
      {
        smtk::resource::PropertiesOfType<double> props = component.properties().get<double>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("intProperties", [](smtk::resource::Component& component)
      {
        smtk::resource::PropertiesOfType<int> props = component.properties().get<int>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("boolProperties", [](smtk::resource::Component& component)
      {
        smtk::resource::PropertiesOfType<bool> props = component.properties().get<bool>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("longProperties", [](smtk::resource::Component& component)
      {
        smtk::resource::PropertiesOfType<long> props = component.properties().get<long>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("stringVectorProperties", [](smtk::resource::Component& component)
      {
        smtk::resource::PropertiesOfType<std::vector<std::string>> props = component.properties().get<std::vector<std::string>>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("doubleVectorProperties", [](smtk::resource::Component& component)
      {
        smtk::resource::PropertiesOfType<std::vector<double>> props = component.properties().get<std::vector<double>>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("intVectorProperties", [](smtk::resource::Component& component)
      {
        smtk::resource::PropertiesOfType<std::vector<int>> props = component.properties().get<std::vector<int>>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("boolVectorProperties", [](smtk::resource::Component& component)
      {
        smtk::resource::PropertiesOfType<std::vector<bool>> props = component.properties().get<std::vector<bool>>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    .def("longVectorProperties", [](smtk::resource::Component& component)
      {
        smtk::resource::PropertiesOfType<std::vector<long>> props = component.properties().get<std::vector<long>>();
        return props;
      }, py::return_value_policy::reference_internal
    )
    ;
  return instance;
}

#endif
