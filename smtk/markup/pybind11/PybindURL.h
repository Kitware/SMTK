//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_markup_URL_h
#define pybind_smtk_markup_URL_h

#include <pybind11/pybind11.h>

#include "smtk/markup/URL.h"

#include "smtk/string/Token.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

namespace py = pybind11;

inline PySharedPtrClass<smtk::markup::URL, smtk::markup::Component> pybind11_init_smtk_markup_URL(py::module &m)
{
  using namespace smtk::string::literals;
  PySharedPtrClass< smtk::markup::URL, smtk::markup::Component> instance(m, "URL");
  instance
    .def("data", [](smtk::markup::URL& self)
      { return self.outgoing("smtk::markup::arcs::URLsToData"_token); },
      py::return_value_policy::reference_internal)
    .def("location", &smtk::markup::URL::location)
    .def("setLocation", &smtk::markup::URL::setLocation, py::arg("location"))
    .def("type", &smtk::markup::URL::type)
    .def("setType", &smtk::markup::URL::setType, py::arg("type"))
    ;
  return instance;
}

#endif
