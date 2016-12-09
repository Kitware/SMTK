//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_SimpleModelSubphrases_h
#define pybind_smtk_model_SimpleModelSubphrases_h

#include <pybind11/pybind11.h>

#include "smtk/model/SimpleModelSubphrases.h"

#include "smtk/model/PropertyType.h"
#include "smtk/model/SubphraseGenerator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::SimpleModelSubphrases, smtk::model::SubphraseGenerator > pybind11_init_smtk_model_SimpleModelSubphrases(py::module &m)
{
  PySharedPtrClass< smtk::model::SimpleModelSubphrases, smtk::model::SubphraseGenerator > instance(m, "SimpleModelSubphrases");
  instance
    .def(py::init<::smtk::model::SimpleModelSubphrases const &>())
    .def("deepcopy", (smtk::model::SimpleModelSubphrases & (smtk::model::SimpleModelSubphrases::*)(::smtk::model::SimpleModelSubphrases const &)) &smtk::model::SimpleModelSubphrases::operator=)
    .def("abridgeUses", &smtk::model::SimpleModelSubphrases::abridgeUses)
    .def("classname", &smtk::model::SimpleModelSubphrases::classname)
    .def_static("create", (std::shared_ptr<smtk::model::SimpleModelSubphrases> (*)()) &smtk::model::SimpleModelSubphrases::create)
    .def_static("create", (std::shared_ptr<smtk::model::SimpleModelSubphrases> (*)(::std::shared_ptr<smtk::model::SimpleModelSubphrases> &)) &smtk::model::SimpleModelSubphrases::create, py::arg("ref"))
    .def("setAbridgeUses", &smtk::model::SimpleModelSubphrases::setAbridgeUses, py::arg("doAbridge"))
    .def("shared_from_this", (std::shared_ptr<const smtk::model::SimpleModelSubphrases> (smtk::model::SimpleModelSubphrases::*)() const) &smtk::model::SimpleModelSubphrases::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::SimpleModelSubphrases> (smtk::model::SimpleModelSubphrases::*)()) &smtk::model::SimpleModelSubphrases::shared_from_this)
    .def("shouldOmitProperty", &smtk::model::SimpleModelSubphrases::shouldOmitProperty, py::arg("parent"), py::arg("ptype"), py::arg("pname"))
    .def("subphrases", &smtk::model::SimpleModelSubphrases::subphrases, py::arg("src"))
    ;
  return instance;
}

#endif
