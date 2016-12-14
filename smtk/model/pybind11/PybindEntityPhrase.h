//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_EntityPhrase_h
#define pybind_smtk_model_EntityPhrase_h

#include <pybind11/pybind11.h>

#include "smtk/model/EntityPhrase.h"

#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::EntityPhrase, smtk::model::DescriptivePhrase > pybind11_init_smtk_model_EntityPhrase(py::module &m)
{
  PySharedPtrClass< smtk::model::EntityPhrase, smtk::model::DescriptivePhrase > instance(m, "EntityPhrase");
  instance
    .def(py::init<::smtk::model::EntityPhrase const &>())
    .def("deepcopy", (smtk::model::EntityPhrase & (smtk::model::EntityPhrase::*)(::smtk::model::EntityPhrase const &)) &smtk::model::EntityPhrase::operator=)
    .def_static("PhrasesFromUUIDs", &smtk::model::EntityPhrase::PhrasesFromUUIDs, py::arg("arg0"), py::arg("arg1"))
    .def("classname", &smtk::model::EntityPhrase::classname)
    .def_static("create", (std::shared_ptr<smtk::model::EntityPhrase> (*)()) &smtk::model::EntityPhrase::create)
    .def_static("create", (std::shared_ptr<smtk::model::EntityPhrase> (*)(::std::shared_ptr<smtk::model::EntityPhrase> &)) &smtk::model::EntityPhrase::create, py::arg("ref"))
    .def("isRelatedColorMutable", &smtk::model::EntityPhrase::isRelatedColorMutable)
    .def("isTitleMutable", &smtk::model::EntityPhrase::isTitleMutable)
    .def("relatedColor", &smtk::model::EntityPhrase::relatedColor)
    .def("relatedEntity", &smtk::model::EntityPhrase::relatedEntity)
    .def("setMutability", &smtk::model::EntityPhrase::setMutability, py::arg("whatsMutable"))
    .def("setRelatedColor", &smtk::model::EntityPhrase::setRelatedColor, py::arg("rgba"))
    .def("setTitle", &smtk::model::EntityPhrase::setTitle, py::arg("newTitle"))
    .def("setup", &smtk::model::EntityPhrase::setup, py::arg("entity"), py::arg("parent") = ::smtk::model::DescriptivePhrasePtr( ))
    .def("shared_from_this", (std::shared_ptr<const smtk::model::EntityPhrase> (smtk::model::EntityPhrase::*)() const) &smtk::model::EntityPhrase::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::EntityPhrase> (smtk::model::EntityPhrase::*)()) &smtk::model::EntityPhrase::shared_from_this)
    .def("subtitle", &smtk::model::EntityPhrase::subtitle)
    .def("title", &smtk::model::EntityPhrase::title)
    ;
  return instance;
}

#endif
