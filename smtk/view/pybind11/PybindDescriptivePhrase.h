//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind__Stage_Source_cmb_5_ThirdParty_SMTK_smtk_view_DescriptivePhrase_h
#define pybind__Stage_Source_cmb_5_ThirdParty_SMTK_smtk_view_DescriptivePhrase_h

#include <pybind11/pybind11.h>

#include "smtk/view/DescriptivePhrase.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/resource/Component.h"
#include "smtk/resource/PropertyType.h"
#include "smtk/resource/Resource.h"
#include "smtk/view/SubphraseGenerator.h"

namespace py = pybind11;

void pybind11_init_smtk_view_DescriptivePhraseType(py::module &m)
{
  py::enum_<smtk::view::DescriptivePhraseType>(m, "DescriptivePhraseType")
    .value("RESOURCE_LIST", smtk::view::DescriptivePhraseType::RESOURCE_LIST)
    .value("RESOURCE_SUMMARY", smtk::view::DescriptivePhraseType::RESOURCE_SUMMARY)
    .value("COMPONENT_LIST", smtk::view::DescriptivePhraseType::COMPONENT_LIST)
    .value("COMPONENT_SUMMARY", smtk::view::DescriptivePhraseType::COMPONENT_SUMMARY)
    .value("PROPERTY_LIST", smtk::view::DescriptivePhraseType::PROPERTY_LIST)
    .value("FLOAT_PROPERTY_VALUE", smtk::view::DescriptivePhraseType::FLOAT_PROPERTY_VALUE)
    .value("STRING_PROPERTY_VALUE", smtk::view::DescriptivePhraseType::STRING_PROPERTY_VALUE)
    .value("INTEGER_PROPERTY_VALUE", smtk::view::DescriptivePhraseType::INTEGER_PROPERTY_VALUE)
    .value("INVALID_DESCRIPTION", smtk::view::DescriptivePhraseType::INVALID_DESCRIPTION)
    .export_values();
}

PySharedPtrClass< smtk::view::DescriptivePhrase > pybind11_init_smtk_view_DescriptivePhrase(py::module &m)
{
  PySharedPtrClass< smtk::view::DescriptivePhrase > instance(m, "DescriptivePhrase");
  instance
    .def(py::init<::smtk::view::DescriptivePhrase const &>())
    .def("deepcopy", (smtk::view::DescriptivePhrase & (smtk::view::DescriptivePhrase::*)(::smtk::view::DescriptivePhrase const &)) &smtk::view::DescriptivePhrase::operator=)
    .def("setup", &smtk::view::DescriptivePhrase::setup, py::arg("phraseType"), py::arg("parent") = ::smtk::view::DescriptivePhrase::Ptr( ))
    .def("setDelegate", &smtk::view::DescriptivePhrase::setDelegate, py::arg("delegate"))
    .def("title", &smtk::view::DescriptivePhrase::title)
    .def("isTitleMutable", &smtk::view::DescriptivePhrase::isTitleMutable)
    .def("setTitle", &smtk::view::DescriptivePhrase::setTitle, py::arg("newTitle"))
    .def("subtitle", &smtk::view::DescriptivePhrase::subtitle)
    .def("isSubtitleMutable", &smtk::view::DescriptivePhrase::isSubtitleMutable)
    .def("setSubtitle", &smtk::view::DescriptivePhrase::setSubtitle, py::arg("newSubtitle"))
    .def("phraseType", &smtk::view::DescriptivePhrase::phraseType)
    .def("parent", &smtk::view::DescriptivePhrase::parent)
    .def("markDirty", &smtk::view::DescriptivePhrase::markDirty, py::arg("dirty") = true)
    .def("areSubphrasesBuilt", &smtk::view::DescriptivePhrase::areSubphrasesBuilt)
    .def("subphrases", (smtk::view::DescriptivePhrases & (smtk::view::DescriptivePhrase::*)()) &smtk::view::DescriptivePhrase::subphrases)
    .def("subphrases", (smtk::view::DescriptivePhrases (smtk::view::DescriptivePhrase::*)() const) &smtk::view::DescriptivePhrase::subphrases)
    .def("argFindChild", (int (smtk::view::DescriptivePhrase::*)(::smtk::view::DescriptivePhrase const *) const) &smtk::view::DescriptivePhrase::argFindChild, py::arg("child"))
    .def("argFindChild", (int (smtk::view::DescriptivePhrase::*)(const ::smtk::resource::ResourcePtr&, bool) const) &smtk::view::DescriptivePhrase::argFindChild, py::arg("child"), py::arg("onlyResource") = true)
    .def("argFindChild", (int (smtk::view::DescriptivePhrase::*)(const ::smtk::resource::ComponentPtr&) const) &smtk::view::DescriptivePhrase::argFindChild, py::arg("child"))
    .def("argFindChild", (int (smtk::view::DescriptivePhrase::*)(::std::string const &, ::smtk::resource::PropertyType) const) &smtk::view::DescriptivePhrase::argFindChild, py::arg("propName"), py::arg("propType"))
    .def("indexInParent", &smtk::view::DescriptivePhrase::indexInParent)
    .def("relatedResource", &smtk::view::DescriptivePhrase::relatedResource)
    .def("relatedComponent", &smtk::view::DescriptivePhrase::relatedComponent)
    .def("relatedComponentId", &smtk::view::DescriptivePhrase::relatedComponentId)
    .def("relatedPropertyName", &smtk::view::DescriptivePhrase::relatedPropertyName)
    .def("relatedPropertyType", &smtk::view::DescriptivePhrase::relatedPropertyType)
    .def("relatedColor", &smtk::view::DescriptivePhrase::relatedColor)
    .def("isRelatedColorMutable", &smtk::view::DescriptivePhrase::isRelatedColorMutable)
    .def("setRelatedColor", &smtk::view::DescriptivePhrase::setRelatedColor, py::arg("rgba"))
    .def("phraseId", &smtk::view::DescriptivePhrase::phraseId)
    .def("findDelegate", &smtk::view::DescriptivePhrase::findDelegate)
    .def("isPropertyValueType", &smtk::view::DescriptivePhrase::isPropertyValueType)
    .def_static("compareByTypeThenTitle", &smtk::view::DescriptivePhrase::compareByTypeThenTitle, py::arg("a"), py::arg("b"))
    .def_static("compareByTitle", &smtk::view::DescriptivePhrase::compareByTitle, py::arg("a"), py::arg("b"))
    ;
  return instance;
}

#endif
