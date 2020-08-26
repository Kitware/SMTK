//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind__Stage_Source_cmb_v4_ThirdParty_SMTK_smtk_model_EntityRef_h
#define pybind__Stage_Source_cmb_v4_ThirdParty_SMTK_smtk_model_EntityRef_h

#include <pybind11/pybind11.h>

#include "smtk/model/EntityRef.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/Arrangement.h"
#include "smtk/model/ArrangementKind.h"
#include "smtk/model/Entity.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Model.h"
#include "smtk/model/SessionRef.h"
#include "smtk/model/StringData.h"
#include "smtk/model/Tessellation.h"

namespace py = pybind11;
void pybind11_init_smtk_model_Exclusions(py::module &m)
{
  py::enum_<smtk::model::Exclusions>(m, "Exclusions")
    .value("Nothing", smtk::model::Exclusions::Nothing)
    .value("Rendering", smtk::model::Exclusions::Rendering)
    .value("ViewPresentation", smtk::model::Exclusions::ViewPresentation)
    .value("Everything", smtk::model::Exclusions::Everything)
    .export_values();
}

py::class_< smtk::model::EntityRef > pybind11_init_smtk_model_EntityRef(py::module &m)
{
  py::class_< smtk::model::EntityRef > instance(m, "EntityRef");
  instance
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::ResourcePtr, ::smtk::common::UUID const &>())
    .def(py::init<::smtk::model::EntityPtr>())
    .def("__ne__", (bool (smtk::model::EntityRef::*)(::smtk::model::EntityRef const &) const) &smtk::model::EntityRef::operator!=)
    .def("__lt__", (bool (smtk::model::EntityRef::*)(::smtk::model::EntityRef const &) const) &smtk::model::EntityRef::operator<)
    .def("deepcopy", (smtk::model::EntityRef & (smtk::model::EntityRef::*)(::smtk::model::EntityRef const &)) &smtk::model::EntityRef::operator=)
    .def("__eq__", (bool (smtk::model::EntityRef::*)(::smtk::model::EntityRef const &) const) &smtk::model::EntityRef::operator==)
    .def("addRawRelation", &smtk::model::EntityRef::addRawRelation, py::arg("ent"))
    .def("adjacentEntities", &smtk::model::EntityRef::adjacentEntities, py::arg("ofDimension"))
    .def("assignDefaultName", &smtk::model::EntityRef::assignDefaultName, py::arg("overwrite") = false)
    .def("associateAttribute", &smtk::model::EntityRef::associateAttribute, py::arg("sys"), py::arg("attribId"))
    .def("attributes", (smtk::attribute::Attributes (smtk::model::EntityRef::*) (smtk::attribute::ConstResourcePtr) const) &smtk::model::EntityRef::attributes)
    .def("attributes", (smtk::attribute::Attributes (smtk::model::EntityRef::*) (smtk::attribute::ConstDefinitionPtr) const) &smtk::model::EntityRef::attributes)
    .def("attributes", (smtk::attribute::Attributes (smtk::model::EntityRef::*) () const) &smtk::model::EntityRef::attributes)
    .def("bordantEntities", &smtk::model::EntityRef::bordantEntities, py::arg("ofDimension") = -2)
    .def("boundaryEntities", &smtk::model::EntityRef::boundaryEntities, py::arg("ofDimension") = -2)
    .def("boundingBox", &smtk::model::EntityRef::boundingBox)
    // .def("checkForArrangements", &smtk::model::EntityRef::checkForArrangements, py::arg("k"), py::arg("entry"), py::arg("arr"))
    .def("clearArrangements", &smtk::model::EntityRef::clearArrangements)
    .def("color", &smtk::model::EntityRef::color)
    .def("component", &smtk::model::EntityRef::component)
    .def("containingGroups", &smtk::model::EntityRef::containingGroups)
    .def("dimension", &smtk::model::EntityRef::dimension)
    .def("dimensionBits", &smtk::model::EntityRef::dimensionBits)
    .def("disassociateAllAttributes", (bool (smtk::model::EntityRef::*) (smtk::attribute::ResourcePtr)) &smtk::model::EntityRef::disassociateAllAttributes, py::arg("attResource"))
    .def("disassociateAllAttributes", (bool (smtk::model::EntityRef::*) (smtk::attribute::ResourcePtr, bool)) &smtk::model::EntityRef::disassociateAllAttributes, py::arg("attResource"), py::arg("reverse"))
    .def("disassociateAttribute", (bool (smtk::model::EntityRef::*) (smtk::attribute::ResourcePtr, const smtk::common::UUID&)) &smtk::model::EntityRef::disassociateAttribute, py::arg("attResource"), py::arg("attribId"))
    .def("disassociateAttribute", (bool (smtk::model::EntityRef::*) (smtk::attribute::ResourcePtr, const smtk::common::UUID&, bool)) &smtk::model::EntityRef::disassociateAttribute, py::arg("attResource"), py::arg("attribId"), py::arg("reverse"))
    .def("elideRawRelation", &smtk::model::EntityRef::elideRawRelation, py::arg("ent"))
    .def("embedEntity", &smtk::model::EntityRef::embedEntity, py::arg("thingToEmbed"), py::arg("checkExists") = true)
    .def("embeddedIn", &smtk::model::EntityRef::embeddedIn)
    .def("embeddingDimension", &smtk::model::EntityRef::embeddingDimension)
    .def("entity", &smtk::model::EntityRef::entity)
    .def("entityFlags", &smtk::model::EntityRef::entityFlags)
    .def("findArrangement", (smtk::model::Arrangement * (smtk::model::EntityRef::*)(::smtk::model::ArrangementKind, int)) &smtk::model::EntityRef::findArrangement, py::arg("k"), py::arg("index"))
    .def("findArrangement", (smtk::model::Arrangement const * (smtk::model::EntityRef::*)(::smtk::model::ArrangementKind, int) const) &smtk::model::EntityRef::findArrangement, py::arg("k"), py::arg("index"))
    .def("findEntitiesWithTessellation", &smtk::model::EntityRef::findEntitiesWithTessellation, py::arg("entityrefMap"), py::arg("touched"))
    .def("findOrAddRawRelation", &smtk::model::EntityRef::findOrAddRawRelation, py::arg("ent"))
    .def("flagSummary", &smtk::model::EntityRef::flagSummary, py::arg("form") = 0)
    .def("floatProperty", (smtk::model::FloatList const & (smtk::model::EntityRef::*)(::std::string const &) const) &smtk::model::EntityRef::floatProperty, py::arg("propName"))
    .def("floatProperty", (smtk::model::FloatList & (smtk::model::EntityRef::*)(::std::string const &)) &smtk::model::EntityRef::floatProperty, py::arg("propName"))
    .def("floatPropertyNames", &smtk::model::EntityRef::floatPropertyNames)
    .def("gotMesh", &smtk::model::EntityRef::gotMesh, py::return_value_policy::reference)
    .def("hasAnalysisMesh", &smtk::model::EntityRef::hasAnalysisMesh, py::return_value_policy::reference)
    .def("hasAttribute", &smtk::model::EntityRef::hasAttribute, py::arg("attribId"))
    .def("hasAttributes", (bool (smtk::model::EntityRef::*) (smtk::attribute::ConstResourcePtr) const) &smtk::model::EntityRef::hasAttributes, py::arg("attributeResource"))
    .def("hasAttributes", (bool (smtk::model::EntityRef::*) () const) &smtk::model::EntityRef::hasAttributes)
    .def("hasColor", &smtk::model::EntityRef::hasColor)
    .def("hasFloatProperties", &smtk::model::EntityRef::hasFloatProperties)
    .def("hasFloatProperty", &smtk::model::EntityRef::hasFloatProperty, py::arg("propName"))
    .def("hasIntegerProperties", &smtk::model::EntityRef::hasIntegerProperties)
    .def("hasIntegerProperty", &smtk::model::EntityRef::hasIntegerProperty, py::arg("propName"))
    .def("hasStringProperties", &smtk::model::EntityRef::hasStringProperties)
    .def("hasStringProperty", &smtk::model::EntityRef::hasStringProperty, py::arg("propName"))
    .def("hasTessellation", &smtk::model::EntityRef::hasTessellation, py::return_value_policy::reference)
    .def("hasVisibility", &smtk::model::EntityRef::hasVisibility)
    .def("hash", &smtk::model::EntityRef::hash)
    .def("exclusions", &smtk::model::EntityRef::exclusions, py::arg("mask") = smtk::model::Exclusions::Everything)
    .def("higherDimensionalBordants", &smtk::model::EntityRef::higherDimensionalBordants, py::arg("higherDimension"))
    .def("instances", [](const smtk::model::EntityRef& prototype) {
      auto result = prototype.instances<smtk::model::Instances>();
      return result; })
    .def("integerProperty", (smtk::model::IntegerList const & (smtk::model::EntityRef::*)(::std::string const &) const) &smtk::model::EntityRef::integerProperty, py::arg("propName"))
    .def("integerProperty", (smtk::model::IntegerList & (smtk::model::EntityRef::*)(::std::string const &)) &smtk::model::EntityRef::integerProperty, py::arg("propName"))
    .def("integerPropertyNames", &smtk::model::EntityRef::integerPropertyNames)
    .def("isAuxiliaryGeometry", &smtk::model::EntityRef::isAuxiliaryGeometry)
    .def("isCellEntity", &smtk::model::EntityRef::isCellEntity)
    .def("isChain", &smtk::model::EntityRef::isChain)
    .def("isConcept", &smtk::model::EntityRef::isConcept)
    .def("isEdge", &smtk::model::EntityRef::isEdge)
    .def("isEdgeUse", &smtk::model::EntityRef::isEdgeUse)
    .def("isEmbedded", &smtk::model::EntityRef::isEmbedded, py::arg("ent"))
    .def("isFace", &smtk::model::EntityRef::isFace)
    .def("isFaceUse", &smtk::model::EntityRef::isFaceUse)
    .def("isGroup", &smtk::model::EntityRef::isGroup)
    .def("isInstance", &smtk::model::EntityRef::isInstance)
    .def("isLoop", &smtk::model::EntityRef::isLoop)
    .def("isModel", &smtk::model::EntityRef::isModel)
    .def("isSessionRef", &smtk::model::EntityRef::isSessionRef)
    .def("isShell", &smtk::model::EntityRef::isShell)
    .def("isShellEntity", &smtk::model::EntityRef::isShellEntity)
    .def("isUseEntity", &smtk::model::EntityRef::isUseEntity)
    .def("isValid", (bool (smtk::model::EntityRef::*)() const) &smtk::model::EntityRef::isValid)
    // .def("isValid", (bool (smtk::model::EntityRef::*)(::smtk::model::Entity * *) const) &smtk::model::EntityRef::isValid, py::arg("entityRecord"))
    .def("isVertex", &smtk::model::EntityRef::isVertex)
    .def("isVertexUse", &smtk::model::EntityRef::isVertexUse)
    .def("isVolume", &smtk::model::EntityRef::isVolume)
    .def("isVolumeUse", &smtk::model::EntityRef::isVolumeUse)
    .def("lowerDimensionalBoundaries", &smtk::model::EntityRef::lowerDimensionalBoundaries, py::arg("lowerDimension"))
    .def("resource", (smtk::model::ResourcePtr (smtk::model::EntityRef::*)()) &smtk::model::EntityRef::resource)
    .def("resource", (smtk::model::ResourcePtr const (smtk::model::EntityRef::*)() const) &smtk::model::EntityRef::resource)
    .def("maxParametricDimension", &smtk::model::EntityRef::maxParametricDimension)
    .def("name", &smtk::model::EntityRef::name)
    .def("numberOfArrangementsOfKind", &smtk::model::EntityRef::numberOfArrangementsOfKind, py::arg("k"))
    .def("owningModel", &smtk::model::EntityRef::owningModel)
    .def("owningSession", &smtk::model::EntityRef::owningSession)
    .def("relationFromArrangement", &smtk::model::EntityRef::relationFromArrangement, py::arg("k"), py::arg("arrangementIndex"), py::arg("offset"))
    .def("relations", &smtk::model::EntityRef::relations)
    .def("removeArrangement", &smtk::model::EntityRef::removeArrangement, py::arg("k"), py::arg("index") = -1)
    .def("removeFloatProperty", &smtk::model::EntityRef::removeFloatProperty, py::arg("propName"))
    .def("removeIntegerProperty", &smtk::model::EntityRef::removeIntegerProperty, py::arg("propName"))
    .def("removeStringProperty", &smtk::model::EntityRef::removeStringProperty, py::arg("propName"))
    .def("removeTessellation", &smtk::model::EntityRef::removeTessellation, py::arg("removeGen") = false)
    .def("resetTessellation", &smtk::model::EntityRef::resetTessellation, py::return_value_policy::reference)
    .def("setBoundingBox", &smtk::model::EntityRef::setBoundingBox, py::arg("bbox"))
    .def("setColor", (void (smtk::model::EntityRef::*)(::smtk::model::FloatList const &)) &smtk::model::EntityRef::setColor, py::arg("rgba"))
    .def("setColor", (void (smtk::model::EntityRef::*)(double, double, double, double)) &smtk::model::EntityRef::setColor, py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a") = 1.)
    .def("setDimensionBits", &smtk::model::EntityRef::setDimensionBits, py::arg("dim"))
    .def("setEntity", &smtk::model::EntityRef::setEntity, py::arg("entityId"))
    .def("setFloatProperty", (void (smtk::model::EntityRef::*)(::std::string const &, ::smtk::model::Float)) &smtk::model::EntityRef::setFloatProperty, py::arg("propName"), py::arg("propValue"))
    .def("setFloatProperty", (void (smtk::model::EntityRef::*)(::std::string const &, ::smtk::model::FloatList const &)) &smtk::model::EntityRef::setFloatProperty, py::arg("propName"), py::arg("propValue"))
    .def("setExclusions", &smtk::model::EntityRef::setExclusions, py::arg("v"), py::arg("mask") = smtk::model::Exclusions::Everything)
    .def("setIntegerProperty", (void (smtk::model::EntityRef::*)(::std::string const &, ::smtk::model::Integer)) &smtk::model::EntityRef::setIntegerProperty, py::arg("propName"), py::arg("propValue"))
    .def("setIntegerProperty", (void (smtk::model::EntityRef::*)(::std::string const &, ::smtk::model::IntegerList const &)) &smtk::model::EntityRef::setIntegerProperty, py::arg("propName"), py::arg("propValue"))
    .def("setResource", &smtk::model::EntityRef::setResource, py::arg("resource"))
    .def("setName", &smtk::model::EntityRef::setName, py::arg("n"))
    .def("setStringProperty", (void (smtk::model::EntityRef::*)(::std::string const &, ::smtk::model::String const &)) &smtk::model::EntityRef::setStringProperty, py::arg("propName"), py::arg("propValue"))
    .def("setStringProperty", (void (smtk::model::EntityRef::*)(::std::string const &, ::smtk::model::StringList const &)) &smtk::model::EntityRef::setStringProperty, py::arg("propName"), py::arg("propValue"))
    .def("setTessellation", &smtk::model::EntityRef::setTessellation, py::arg("tess"), py::arg("analysisMesh") = 0, py::arg("updateBBox") = false)
    .def("setTessellationAndBoundingBox", &smtk::model::EntityRef::setTessellationAndBoundingBox, py::arg("tess"), py::arg("analysisMesh") = 0)
    .def("setVisible", &smtk::model::EntityRef::setVisible, py::arg("vis"))
    .def("stringProperty", (smtk::model::StringList const & (smtk::model::EntityRef::*)(::std::string const &) const) &smtk::model::EntityRef::stringProperty, py::arg("propName"))
    .def("stringProperty", (smtk::model::StringList & (smtk::model::EntityRef::*)(::std::string const &)) &smtk::model::EntityRef::stringProperty, py::arg("propName"))
    .def("stringPropertyNames", &smtk::model::EntityRef::stringPropertyNames)
    .def("tessellationGeneration", &smtk::model::EntityRef::tessellationGeneration)
    .def("unembedEntity", &smtk::model::EntityRef::unembedEntity, py::arg("thingToUnembed"))
    .def("unionBoundingBox", (std::vector<double> (smtk::model::EntityRef::*)(const std::vector<double>&, const std::vector<double>&) const) &smtk::model::EntityRef::unionBoundingBox, py::arg("b1"), py::arg("b2"))
    .def("visible", &smtk::model::EntityRef::visible)
    ;
  return instance;
}

void pybind11_init_smtk_model_entityrefHash(py::module &m)
{
  m.def("entityrefHash", &smtk::model::entityrefHash, "", py::arg("c"));
}

#endif
