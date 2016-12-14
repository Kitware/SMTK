//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_PropertyValuePhrase_h
#define pybind_smtk_model_PropertyValuePhrase_h

#include <pybind11/pybind11.h>

#include "smtk/model/PropertyValuePhrase.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/PropertyType.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::PropertyValuePhrase, smtk::model::DescriptivePhrase > pybind11_init_smtk_model_PropertyValuePhrase(py::module &m)
{
  PySharedPtrClass< smtk::model::PropertyValuePhrase, smtk::model::DescriptivePhrase > instance(m, "PropertyValuePhrase");
  instance
    .def(py::init<::smtk::model::PropertyValuePhrase const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::model::PropertyValuePhrase & (smtk::model::PropertyValuePhrase::*)(::smtk::model::PropertyValuePhrase const &)) &smtk::model::PropertyValuePhrase::operator=)
    .def("classname", &smtk::model::PropertyValuePhrase::classname)
    .def_static("create", (std::shared_ptr<smtk::model::PropertyValuePhrase> (*)()) &smtk::model::PropertyValuePhrase::create)
    .def_static("create", (std::shared_ptr<smtk::model::PropertyValuePhrase> (*)(::std::shared_ptr<smtk::model::PropertyValuePhrase> &)) &smtk::model::PropertyValuePhrase::create, py::arg("ref"))
    .def_static("propertyToPhraseType", &smtk::model::PropertyValuePhrase::propertyToPhraseType, py::arg("p"))
    .def("relatedEntity", &smtk::model::PropertyValuePhrase::relatedEntity)
    .def("relatedEntityId", &smtk::model::PropertyValuePhrase::relatedEntityId)
    .def("relatedPropertyName", &smtk::model::PropertyValuePhrase::relatedPropertyName)
    .def("relatedPropertyType", &smtk::model::PropertyValuePhrase::relatedPropertyType)
    .def("setup", &smtk::model::PropertyValuePhrase::setup, py::arg("propType"), py::arg("propName"), py::arg("parent"))
    .def("subtitle", &smtk::model::PropertyValuePhrase::subtitle)
    .def("title", &smtk::model::PropertyValuePhrase::title)
    ;
  return instance;
}

#endif
