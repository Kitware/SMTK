//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_Properties_h
#define pybind_smtk_resource_Properties_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/resource/Properties.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

namespace py = pybind11;

template<typename ValueType>
inline py::class_< smtk::resource::PropertiesOfType<ValueType> > pybind11_init_smtk_resource_PropertiesOfType(py::module &m)
{
  std::string typeName = "PropertiesOfType_" + smtk::common::typeName<ValueType>();
  py::class_< smtk::resource::PropertiesOfType<ValueType> > instance(m, typeName.c_str());
  instance
    .def("keys", &smtk::resource::PropertiesOfType<ValueType>::keys)
    .def("insert", &smtk::resource::PropertiesOfType<ValueType>::insert, py::arg("name"), py::arg("value"))
    .def("contains", &smtk::resource::PropertiesOfType<ValueType>::contains, py::arg("name"))
    .def("erase", &smtk::resource::PropertiesOfType<ValueType>::erase, py::arg("name"))
    .def("insert", (bool(smtk::resource::PropertiesOfType<ValueType>::*)(const std::string&, const ValueType&))&smtk::resource::PropertiesOfType<ValueType>::insert, py::arg("name"), py::arg("value"))
    .def("at", (ValueType& (smtk::resource::PropertiesOfType<ValueType>::*)(const std::string&))&smtk::resource::PropertiesOfType<ValueType>::at, py::arg("name"))
    .def("set", [](smtk::resource::PropertiesOfType<ValueType>& properties, const std::string& name, ValueType& value)
      {
        if (properties.contains(name))
        {
          properties.at(name) = value;
        }
        else
        {
          properties.insert(name, value);
        }
      }, py::arg("name"), py::arg("value"))
    .def("empty", &smtk::resource::PropertiesOfType<ValueType>::empty)
    ;
  return instance;
}

#endif
