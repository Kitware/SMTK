//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind__Stage_Source_cmb_5_ThirdParty_SMTK_smtk_view_SubphraseGenerator_h
#define pybind__Stage_Source_cmb_5_ThirdParty_SMTK_smtk_view_SubphraseGenerator_h

#include <pybind11/pybind11.h>

#include "smtk/view/SubphraseGenerator.h"

#include "smtk/resource/PropertyType.h"

namespace py = pybind11;

PySharedPtrClass< smtk::view::SubphraseGenerator > pybind11_init_smtk_view_SubphraseGenerator(py::module &m)
{
  PySharedPtrClass< smtk::view::SubphraseGenerator > instance(m, "SubphraseGenerator");
  instance
    .def(py::init<::smtk::view::SubphraseGenerator const &>())
    .def("deepcopy", (smtk::view::SubphraseGenerator & (smtk::view::SubphraseGenerator::*)(::smtk::view::SubphraseGenerator const &)) &smtk::view::SubphraseGenerator::operator=)
    .def("directLimit", &smtk::view::SubphraseGenerator::directLimit)
    .def("setDirectLimit", &smtk::view::SubphraseGenerator::setDirectLimit, py::arg("val"))
    .def("setSkipAttributes", &smtk::view::SubphraseGenerator::setSkipAttributes, py::arg("val"))
    .def("setSkipProperties", &smtk::view::SubphraseGenerator::setSkipProperties, py::arg("val"))
    .def("shouldOmitProperty", &smtk::view::SubphraseGenerator::shouldOmitProperty, py::arg("parent"), py::arg("ptype"), py::arg("pname"))
    .def("skipAttributes", &smtk::view::SubphraseGenerator::skipAttributes)
    .def("skipProperties", &smtk::view::SubphraseGenerator::skipProperties)
    .def("subphrases", &smtk::view::SubphraseGenerator::subphrases, py::arg("src"))
    ;
  return instance;
}

#endif
