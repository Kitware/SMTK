//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_EntityTypeSubphrases_h
#define pybind_smtk_model_EntityTypeSubphrases_h

#include <pybind11/pybind11.h>

#include "smtk/model/EntityTypeSubphrases.h"

#include "smtk/model/PropertyType.h"
#include "smtk/model/SubphraseGenerator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::EntityTypeSubphrases, smtk::model::SubphraseGenerator > pybind11_init_smtk_model_EntityTypeSubphrases(py::module &m)
{
  PySharedPtrClass< smtk::model::EntityTypeSubphrases, smtk::model::SubphraseGenerator > instance(m, "EntityTypeSubphrases");
  instance
    .def(py::init<::smtk::model::EntityTypeSubphrases const &>())
    .def("deepcopy", (smtk::model::EntityTypeSubphrases & (smtk::model::EntityTypeSubphrases::*)(::smtk::model::EntityTypeSubphrases const &)) &smtk::model::EntityTypeSubphrases::operator=)
    .def("classname", &smtk::model::EntityTypeSubphrases::classname)
    .def_static("create", (std::shared_ptr<smtk::model::EntityTypeSubphrases> (*)()) &smtk::model::EntityTypeSubphrases::create)
    .def_static("create", (std::shared_ptr<smtk::model::EntityTypeSubphrases> (*)(::std::shared_ptr<smtk::model::EntityTypeSubphrases> &)) &smtk::model::EntityTypeSubphrases::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::model::EntityTypeSubphrases> (smtk::model::EntityTypeSubphrases::*)() const) &smtk::model::EntityTypeSubphrases::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::EntityTypeSubphrases> (smtk::model::EntityTypeSubphrases::*)()) &smtk::model::EntityTypeSubphrases::shared_from_this)
    .def("shouldOmitProperty", &smtk::model::EntityTypeSubphrases::shouldOmitProperty, py::arg("parent"), py::arg("ptype"), py::arg("pname"))
    .def("subphrases", &smtk::model::EntityTypeSubphrases::subphrases, py::arg("src"))
    ;
  return instance;
}

#endif
