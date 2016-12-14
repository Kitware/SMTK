//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_DescriptivePhrase_h
#define pybind_smtk_model_DescriptivePhrase_h

#include <pybind11/pybind11.h>

#include "smtk/model/DescriptivePhrase.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/model/ArrangementKind.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/PropertyType.h"
#include "smtk/model/SubphraseGenerator.h"

namespace py = pybind11;

void pybind11_init_smtk_model_DescriptivePhraseType(py::module &m)
{
  py::enum_<smtk::model::DescriptivePhraseType>(m, "DescriptivePhraseType")
    .value("ENTITY_LIST", smtk::model::DescriptivePhraseType::ENTITY_LIST)
    .value("ENTITY_SUMMARY", smtk::model::DescriptivePhraseType::ENTITY_SUMMARY)
    .value("ARRANGEMENT_LIST", smtk::model::DescriptivePhraseType::ARRANGEMENT_LIST)
    .value("ATTRIBUTE_LIST", smtk::model::DescriptivePhraseType::ATTRIBUTE_LIST)
    .value("FLOAT_PROPERTY_LIST", smtk::model::DescriptivePhraseType::FLOAT_PROPERTY_LIST)
    .value("STRING_PROPERTY_LIST", smtk::model::DescriptivePhraseType::STRING_PROPERTY_LIST)
    .value("INTEGER_PROPERTY_LIST", smtk::model::DescriptivePhraseType::INTEGER_PROPERTY_LIST)
    .value("ENTITY_HAS_FLOAT_PROPERTY", smtk::model::DescriptivePhraseType::ENTITY_HAS_FLOAT_PROPERTY)
    .value("ENTITY_HAS_STRING_PROPERTY", smtk::model::DescriptivePhraseType::ENTITY_HAS_STRING_PROPERTY)
    .value("ENTITY_HAS_INTEGER_PROPERTY", smtk::model::DescriptivePhraseType::ENTITY_HAS_INTEGER_PROPERTY)
    .value("ATTRIBUTE_ASSOCIATION", smtk::model::DescriptivePhraseType::ATTRIBUTE_ASSOCIATION)
    .value("FLOAT_PROPERTY_VALUE", smtk::model::DescriptivePhraseType::FLOAT_PROPERTY_VALUE)
    .value("STRING_PROPERTY_VALUE", smtk::model::DescriptivePhraseType::STRING_PROPERTY_VALUE)
    .value("INTEGER_PROPERTY_VALUE", smtk::model::DescriptivePhraseType::INTEGER_PROPERTY_VALUE)
    .value("ENTITY_HAS_SUBPHRASES", smtk::model::DescriptivePhraseType::ENTITY_HAS_SUBPHRASES)
    .value("MESH_SUMMARY", smtk::model::DescriptivePhraseType::MESH_SUMMARY)
    .value("MESH_LIST", smtk::model::DescriptivePhraseType::MESH_LIST)
    .value("INVALID_DESCRIPTION", smtk::model::DescriptivePhraseType::INVALID_DESCRIPTION)
    .export_values();
}

PySharedPtrClass< smtk::model::DescriptivePhrase > pybind11_init_smtk_model_DescriptivePhrase(py::module &m)
{
  PySharedPtrClass< smtk::model::DescriptivePhrase > instance(m, "DescriptivePhrase");
  instance
    .def(py::init<::smtk::model::DescriptivePhrase const &>())
    .def("deepcopy", (smtk::model::DescriptivePhrase & (smtk::model::DescriptivePhrase::*)(::smtk::model::DescriptivePhrase const &)) &smtk::model::DescriptivePhrase::operator=)
    .def("areSubphrasesBuilt", &smtk::model::DescriptivePhrase::areSubphrasesBuilt)
    .def("argFindChild", (int (smtk::model::DescriptivePhrase::*)(::smtk::model::DescriptivePhrase const *) const) &smtk::model::DescriptivePhrase::argFindChild, py::arg("child"))
    .def("argFindChild", (int (smtk::model::DescriptivePhrase::*)(::smtk::model::EntityRef const &) const) &smtk::model::DescriptivePhrase::argFindChild, py::arg("child"))
    .def("argFindChild", (int (smtk::model::DescriptivePhrase::*)(::smtk::mesh::MeshSet const &) const) &smtk::model::DescriptivePhrase::argFindChild, py::arg("child"))
    .def("argFindChild", (int (smtk::model::DescriptivePhrase::*)(::smtk::mesh::CollectionPtr const &) const) &smtk::model::DescriptivePhrase::argFindChild, py::arg("child"))
    .def("argFindChild", (int (smtk::model::DescriptivePhrase::*)(::std::string const &, ::smtk::model::PropertyType) const) &smtk::model::DescriptivePhrase::argFindChild, py::arg("propName"), py::arg("propType"))
    .def("classname", &smtk::model::DescriptivePhrase::classname)
    .def("findDelegate", &smtk::model::DescriptivePhrase::findDelegate)
    .def("indexInParent", &smtk::model::DescriptivePhrase::indexInParent)
    .def("isPropertyValueType", &smtk::model::DescriptivePhrase::isPropertyValueType)
    .def("isRelatedColorMutable", &smtk::model::DescriptivePhrase::isRelatedColorMutable)
    .def("isSubtitleMutable", &smtk::model::DescriptivePhrase::isSubtitleMutable)
    .def("isTitleMutable", &smtk::model::DescriptivePhrase::isTitleMutable)
    .def("markDirty", &smtk::model::DescriptivePhrase::markDirty, py::arg("dirty") = true)
    .def("parent", &smtk::model::DescriptivePhrase::parent)
    .def("phraseId", &smtk::model::DescriptivePhrase::phraseId)
    .def("phraseType", &smtk::model::DescriptivePhrase::phraseType)
    .def("relatedArrangementKind", &smtk::model::DescriptivePhrase::relatedArrangementKind)
    .def("relatedAttributeId", &smtk::model::DescriptivePhrase::relatedAttributeId)
    .def("relatedColor", &smtk::model::DescriptivePhrase::relatedColor)
    .def("relatedEntity", &smtk::model::DescriptivePhrase::relatedEntity)
    .def("relatedEntityId", &smtk::model::DescriptivePhrase::relatedEntityId)
    .def("relatedMesh", &smtk::model::DescriptivePhrase::relatedMesh)
    .def("relatedMeshCollection", &smtk::model::DescriptivePhrase::relatedMeshCollection)
    .def("relatedPropertyName", &smtk::model::DescriptivePhrase::relatedPropertyName)
    .def("relatedPropertyType", &smtk::model::DescriptivePhrase::relatedPropertyType)
    .def("setDelegate", &smtk::model::DescriptivePhrase::setDelegate, py::arg("delegate"))
    .def("setRelatedColor", &smtk::model::DescriptivePhrase::setRelatedColor, py::arg("rgba"))
    .def("setSubtitle", &smtk::model::DescriptivePhrase::setSubtitle, py::arg("newSubtitle"))
    .def("setTitle", &smtk::model::DescriptivePhrase::setTitle, py::arg("newTitle"))
    .def("setup", &smtk::model::DescriptivePhrase::setup, py::arg("phraseType"), py::arg("parent") = ::smtk::model::DescriptivePhrase::Ptr( ))
    .def("subphrases", (smtk::model::DescriptivePhrases & (smtk::model::DescriptivePhrase::*)()) &smtk::model::DescriptivePhrase::subphrases)
    .def("subphrases", (smtk::model::DescriptivePhrases (smtk::model::DescriptivePhrase::*)() const) &smtk::model::DescriptivePhrase::subphrases)
    .def("subtitle", &smtk::model::DescriptivePhrase::subtitle)
    .def("title", &smtk::model::DescriptivePhrase::title)
    ;
  return instance;
}

#endif
