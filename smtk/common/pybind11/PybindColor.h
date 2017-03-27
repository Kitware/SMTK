//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_Color_h
#define pybind_smtk_common_Color_h

#include <pybind11/pybind11.h>

#include "smtk/common/Color.h"

namespace py = pybind11;

py::class_< smtk::common::Color > pybind11_init_smtk_common_Color(py::module &m)
{
  py::class_< smtk::common::Color > instance(m, "Color");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::common::Color const &>())
    .def("deepcopy",
      (smtk::common::Color & (smtk::common::Color::*)(::smtk::common::Color const &))
      &smtk::common::Color::operator=)
    .def_static("stringToFloatRGBA",
      (bool (*)(::std::vector<double, std::allocator<double> > &, ::std::string const &))
      &smtk::common::Color::stringToFloatRGBA,
      py::arg("rgba"), py::arg("colorSpec"))
    ;
  return instance;
}

#endif
