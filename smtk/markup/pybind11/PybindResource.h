//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_markup_Resource_h
#define pybind_smtk_markup_Resource_h

#include <pybind11/pybind11.h>

#include "smtk/markup/Resource.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/geometry/Resource.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::markup::Resource> pybind11_init_smtk_markup_Resource(py::module &m)
{
  PySharedPtrClass< smtk::markup::Resource, smtk::graph::ResourceBase> instance(m, "Resource");
  instance
    .def_static("create", []() { return smtk::markup::Resource::create(); })
    .def("domains", [](smtk::markup::Resource& rr) { return rr.domains(); })
    .def("lengthUnit", &smtk::markup::Resource::lengthUnit)
    .def("setLengthUnit", &smtk::markup::Resource::setLengthUnit, py::arg("lengthUnit"))
    ;
  return instance;
}

#endif
