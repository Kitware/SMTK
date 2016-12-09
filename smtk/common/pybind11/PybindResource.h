//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_Resource_h
#define pybind_smtk_common_Resource_h

#include <pybind11/pybind11.h>

#include "smtk/common/Resource.h"

namespace py = pybind11;

PySharedPtrClass< smtk::common::Resource > pybind11_init_smtk_common_Resource(py::module &m)
{
  PySharedPtrClass< smtk::common::Resource > instance(m, "Resource");
  instance
    .def("deepcopy", (smtk::common::Resource & (smtk::common::Resource::*)(::smtk::common::Resource const &)) &smtk::common::Resource::operator=)
    .def("resourceType", &smtk::common::Resource::resourceType)
    .def_static("type2String", &smtk::common::Resource::type2String, py::arg("t"))
    .def_static("string2Type", &smtk::common::Resource::string2Type, py::arg("s"))
    ;
  py::enum_<smtk::common::Resource::Type>(instance, "Type")
    .value("ATTRIBUTE", smtk::common::Resource::Type::ATTRIBUTE)
    .value("MODEL", smtk::common::Resource::Type::MODEL)
    .value("MESH", smtk::common::Resource::Type::MESH)
    .value("NUMBER_OF_TYPES", smtk::common::Resource::Type::NUMBER_OF_TYPES)
    .export_values();
  return instance;
}

#endif
