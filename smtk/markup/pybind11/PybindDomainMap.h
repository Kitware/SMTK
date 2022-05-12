//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_markup_DomainMap_h
#define pybind_smtk_markup_DomainMap_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/markup/DomainMap.h"

namespace py = pybind11;

inline PySharedPtrClass<smtk::markup::DomainMap> pybind11_init_smtk_markup_DomainMap(py::module &m)
{
  PySharedPtrClass<smtk::markup::DomainMap> instance(m, "DomainMap");
  instance
    .def("find", [](smtk::markup::DomainMap& self, const std::string& name) { return self.find(name); }, py::arg("name"))
    .def("keys", [](const smtk::markup::DomainMap& self) { return self.keys(); })
    // .def("insert", &smtk::markup::DomainMap::insert, py::arg("domain"))
    ;
  return instance;
}

#endif
