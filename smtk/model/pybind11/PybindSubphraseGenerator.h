//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_SubphraseGenerator_h
#define pybind_smtk_model_SubphraseGenerator_h

#include <pybind11/pybind11.h>

#include "smtk/model/SubphraseGenerator.h"

#include "smtk/model/PropertyType.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::SubphraseGenerator > pybind11_init_smtk_model_SubphraseGenerator(py::module &m)
{
  PySharedPtrClass< smtk::model::SubphraseGenerator > instance(m, "SubphraseGenerator");
  instance
    .def(py::init<::smtk::model::SubphraseGenerator const &>())
    .def("deepcopy", (smtk::model::SubphraseGenerator & (smtk::model::SubphraseGenerator::*)(::smtk::model::SubphraseGenerator const &)) &smtk::model::SubphraseGenerator::operator=)
    .def("classname", &smtk::model::SubphraseGenerator::classname)
    .def("directLimit", &smtk::model::SubphraseGenerator::directLimit)
    .def("setDirectLimit", &smtk::model::SubphraseGenerator::setDirectLimit, py::arg("val"))
    .def("setSkipAttributes", &smtk::model::SubphraseGenerator::setSkipAttributes, py::arg("val"))
    .def("setSkipProperties", &smtk::model::SubphraseGenerator::setSkipProperties, py::arg("val"))
    .def("shouldOmitProperty", &smtk::model::SubphraseGenerator::shouldOmitProperty, py::arg("parent"), py::arg("ptype"), py::arg("pname"))
    .def("skipAttributes", &smtk::model::SubphraseGenerator::skipAttributes)
    .def("skipProperties", &smtk::model::SubphraseGenerator::skipProperties)
    .def("subphrases", &smtk::model::SubphraseGenerator::subphrases, py::arg("src"))
    .def("activeModel", &smtk::model::SubphraseGenerator::activeModel)
    .def("setActiveModel", &smtk::model::SubphraseGenerator::setActiveModel, py::arg("activeModel"))
    ;
  return instance;
}

#endif
