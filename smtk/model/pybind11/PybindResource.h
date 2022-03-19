//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Resource_h
#define pybind_smtk_model_Resource_h

#include <pybind11/pybind11.h>

#include "smtk/model/Resource.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/io/Logger.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/model/Arrangement.h"
#include "smtk/model/ArrangementKind.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Chain.h"
#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"
#include "smtk/model/SessionRef.h"
#include "smtk/model/Shell.h"
#include "smtk/model/StringData.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/VertexUse.h"
#include "smtk/model/Volume.h"
#include "smtk/model/VolumeUse.h"
#include "smtk/resource/Resource.h"
#include "smtk/resource/Component.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::model::Resource> pybind11_init_smtk_model_Resource(py::module &m)
{
  PySharedPtrClass< smtk::model::Resource, smtk::resource::Resource > instance(m, "Resource");
  instance
    .def(py::init<>())
    .def("addAuxiliaryGeometry", (smtk::model::AuxiliaryGeometry (smtk::model::Resource::*)(int)) &smtk::model::Resource::addAuxiliaryGeometry, py::arg("dim") = -1)
    .def("addAuxiliaryGeometry", (smtk::model::AuxiliaryGeometry (smtk::model::Resource::*)(::smtk::model::Model const &, int)) &smtk::model::Resource::addAuxiliaryGeometry, py::arg("parent"), py::arg("dim") = -1)
    .def("addAuxiliaryGeometry", (smtk::model::AuxiliaryGeometry (smtk::model::Resource::*)(::smtk::model::AuxiliaryGeometry const &, int)) &smtk::model::Resource::addAuxiliaryGeometry, py::arg("parent"), py::arg("dim") = -1)
    .def("addCellOfDimension", &smtk::model::Resource::addCellOfDimension, py::arg("dim"))
    .def("addCellOfDimensionWithUUID", &smtk::model::Resource::addCellOfDimensionWithUUID, py::arg("uid"), py::arg("dim"))
    .def("addChain", (smtk::model::Chain (smtk::model::Resource::*)()) &smtk::model::Resource::addChain)
    .def("addChain", (smtk::model::Chain (smtk::model::Resource::*)(::smtk::model::EdgeUse const &)) &smtk::model::Resource::addChain, py::arg("arg0"))
    .def("addChain", (smtk::model::Chain (smtk::model::Resource::*)(::smtk::model::Chain const &)) &smtk::model::Resource::addChain, py::arg("arg0"))
    .def("addDualArrangement", &smtk::model::Resource::addDualArrangement, py::arg("parent"), py::arg("child"), py::arg("kind"), py::arg("sense"), py::arg("orientation"))
    .def("addEdge", &smtk::model::Resource::addEdge)
    .def("addEdgeUse", (smtk::model::EdgeUse (smtk::model::Resource::*)()) &smtk::model::Resource::addEdgeUse)
    .def("addEdgeUse", (smtk::model::EdgeUse (smtk::model::Resource::*)(::smtk::model::Edge const &, int, ::smtk::model::Orientation)) &smtk::model::Resource::addEdgeUse, py::arg("src"), py::arg("sense"), py::arg("o"))
    .def("addEntity", &smtk::model::Resource::addEntity, py::arg("cell"))
    .def("addEntityOfTypeAndDimension", &smtk::model::Resource::addEntityOfTypeAndDimension, py::arg("entityFlags"), py::arg("dim"))
    .def("addEntityOfTypeAndDimensionWithUUID", &smtk::model::Resource::addEntityOfTypeAndDimensionWithUUID, py::arg("uid"), py::arg("entityFlags"), py::arg("dim"))
    .def("addEntityWithUUID", &smtk::model::Resource::addEntityWithUUID, py::arg("uid"), py::arg("cell"))
    .def("addFace", &smtk::model::Resource::addFace)
    .def("addFaceUse", (smtk::model::FaceUse (smtk::model::Resource::*)()) &smtk::model::Resource::addFaceUse)
    .def("addFaceUse", (smtk::model::FaceUse (smtk::model::Resource::*)(::smtk::model::Face const &, int, ::smtk::model::Orientation)) &smtk::model::Resource::addFaceUse, py::arg("src"), py::arg("sense"), py::arg("o"))
    .def("addGroup", &smtk::model::Resource::addGroup, py::arg("extraFlags") = 0, py::arg("name") = std::string())
    .def("addInstance", (smtk::model::Instance (smtk::model::Resource::*)()) &smtk::model::Resource::addInstance)
    .def("addInstance", (smtk::model::Instance (smtk::model::Resource::*)(::smtk::model::EntityRef const &)) &smtk::model::Resource::addInstance, py::arg("instanceOf"))
    .def("addLoop", (smtk::model::Loop (smtk::model::Resource::*)()) &smtk::model::Resource::addLoop)
    .def("addLoop", (smtk::model::Loop (smtk::model::Resource::*)(::smtk::model::FaceUse const &)) &smtk::model::Resource::addLoop, py::arg("arg0"))
    .def("addLoop", (smtk::model::Loop (smtk::model::Resource::*)(::smtk::model::Loop const &)) &smtk::model::Resource::addLoop, py::arg("arg0"))
    .def("addModel", &smtk::model::Resource::addModel, py::arg("parametricDim") = 3, py::arg("embeddingDim") = 3, py::arg("name") = std::string())
    .def("addShell", (smtk::model::Shell (smtk::model::Resource::*)()) &smtk::model::Resource::addShell)
    .def("addShell", (smtk::model::Shell (smtk::model::Resource::*)(::smtk::model::Volume const &)) &smtk::model::Resource::addShell, py::arg("src"))
    .def("addShell", (smtk::model::Shell (smtk::model::Resource::*)(::smtk::model::VolumeUse const &)) &smtk::model::Resource::addShell, py::arg("src"))
    .def("addToGroup", &smtk::model::Resource::addToGroup, py::arg("groupId"), py::arg("uids"))
    .def("addVertex", &smtk::model::Resource::addVertex)
    .def("addVertexUse", (smtk::model::VertexUse (smtk::model::Resource::*)()) &smtk::model::Resource::addVertexUse)
    .def("addVertexUse", (smtk::model::VertexUse (smtk::model::Resource::*)(::smtk::model::Vertex const &, int)) &smtk::model::Resource::addVertexUse, py::arg("src"), py::arg("sense"))
    .def("addVolume", &smtk::model::Resource::addVolume)
    .def("addVolumeUse", (smtk::model::VolumeUse (smtk::model::Resource::*)()) &smtk::model::Resource::addVolumeUse)
    .def("addVolumeUse", (smtk::model::VolumeUse (smtk::model::Resource::*)(::smtk::model::Volume const &)) &smtk::model::Resource::addVolumeUse, py::arg("src"))
    .def("adjacentEntities", &smtk::model::Resource::adjacentEntities, py::arg("ofEntity"), py::arg("ofDimension"))
    .def("analysisMesh", (smtk::model::UUIDsToTessellations & (smtk::model::Resource::*)()) &smtk::model::Resource::analysisMesh)
    .def("analysisMesh", (smtk::model::UUIDsToTessellations const & (smtk::model::Resource::*)() const) &smtk::model::Resource::analysisMesh)
    .def("arrangeEntity", &smtk::model::Resource::arrangeEntity, py::arg("entityId"), py::arg("arg1"), py::arg("arr"), py::arg("index") = -1)
    .def("arrangementsOfKindForEntity", &smtk::model::Resource::arrangementsOfKindForEntity, py::arg("cellId"), py::arg("arg1"))
    .def("assignDefaultName", (std::string (smtk::model::Resource::*)(::smtk::common::UUID const &)) &smtk::model::Resource::assignDefaultName, py::arg("uid"))
    .def("assignDefaultNameIfMissing", &smtk::model::Resource::assignDefaultNameIfMissing, py::arg("uid"))
    .def("assignDefaultNames", &smtk::model::Resource::assignDefaultNames)
    .def("assignDefaultNamesToModelChildren", &smtk::model::Resource::assignDefaultNamesToModelChildren, py::arg("modelId"))
    .def("associateAttribute", &smtk::model::Resource::associateAttribute, py::arg("sys"), py::arg("attribId"), py::arg("toEntity"))
    .def("attributeAssignments", &smtk::model::Resource::attributeAssignments)
    .def("bordantEntities", (smtk::common::UUIDs (smtk::model::Resource::*)(::smtk::common::UUID const &, int) const) &smtk::model::Resource::bordantEntities, py::arg("ofEntity"), py::arg("ofDimension") = -2)
    .def("bordantEntities", (smtk::common::UUIDs (smtk::model::Resource::*)(::smtk::common::UUIDs const &, int) const) &smtk::model::Resource::bordantEntities, py::arg("ofEntities"), py::arg("ofDimension") = -2)
    .def("boundaryEntities", (smtk::common::UUIDs (smtk::model::Resource::*)(::smtk::common::UUID const &, int) const) &smtk::model::Resource::boundaryEntities, py::arg("ofEntity"), py::arg("ofDimension") = -2)
    .def("boundaryEntities", (smtk::common::UUIDs (smtk::model::Resource::*)(::smtk::common::UUIDs const &, int) const) &smtk::model::Resource::boundaryEntities, py::arg("ofEntities"), py::arg("ofDimension") = -2)
    .def("cellHasUseOfSenseAndOrientation", &smtk::model::Resource::cellHasUseOfSenseAndOrientation, py::arg("cell"), py::arg("sense"), py::arg("o"))
    .def("clearArrangements", &smtk::model::Resource::clearArrangements, py::arg("entityId"))
    .def("closeSession", &smtk::model::Resource::closeSession, py::arg("sess"))
    .def_static("create", (std::shared_ptr<smtk::model::Resource> (*)()) &smtk::model::Resource::create)
    .def_static("create", (std::shared_ptr<smtk::model::Resource> (*)(::std::shared_ptr<smtk::model::Resource> &)) &smtk::model::Resource::create, py::arg("ref"))
    .def("createIncludedShell", &smtk::model::Resource::createIncludedShell, py::arg("cellUseOrShell"))
    .def("dimension", &smtk::model::Resource::dimension, py::arg("ofEntity"))
    .def("disassociateAttribute", &smtk::model::Resource::disassociateAttribute, py::arg("sys"), py::arg("attribId"), py::arg("fromEntity"), py::arg("reverse") = true)
    .def("elideEntityReferences", &smtk::model::Resource::elideEntityReferences, py::arg("c"))
    .def("elideOneEntityReference", &smtk::model::Resource::elideOneEntityReference, py::arg("c"), py::arg("r"))
    .def("entitiesMatchingFlags", &smtk::model::Resource::entitiesMatchingFlags, py::arg("mask"), py::arg("exactMatch") = true)
    .def("entitiesOfDimension", &smtk::model::Resource::entitiesOfDimension, py::arg("dim"))
    .def("erase", (smtk::model::SessionInfoBits (smtk::model::Resource::*)(::smtk::common::UUID const &, ::smtk::model::SessionInfoBits)) &smtk::model::Resource::erase, py::arg("uid"), py::arg("flags") = ::smtk::model::SessionInfoBits(::smtk::model::SessionInformation::SESSION_EVERYTHING))
    .def("erase", (smtk::model::SessionInfoBits (smtk::model::Resource::*)(::smtk::model::EntityRef const &, ::smtk::model::SessionInfoBits)) &smtk::model::Resource::erase, py::arg("entityref"), py::arg("flags") = ::smtk::model::SessionInfoBits(::smtk::model::SessionInformation::SESSION_EVERYTHING))
    .def("eraseModel", &smtk::model::Resource::eraseModel, py::arg("entityref"), py::arg("flags") = ::smtk::model::SessionInfoBits(::smtk::model::SessionInformation::SESSION_EVERYTHING))
    .def("find", (smtk::resource::ComponentPtr (smtk::model::Resource::*)(const smtk::common::UUID&) const) &smtk::model::Resource::find)
    .def("queryOperation", &smtk::model::Resource::queryOperation)
    .def("visit", (void (smtk::model::Resource::*)(std::function<void(const smtk::resource::ComponentPtr&)>&) const) &smtk::model::Resource::visit)
    .def("findArrangement", (smtk::model::Arrangement const * (smtk::model::Resource::*)(::smtk::common::UUID const &, ::smtk::model::ArrangementKind, int) const) &smtk::model::Resource::findArrangement, py::arg("entityId"), py::arg("kind"), py::arg("index"))
    .def("findArrangement", (smtk::model::Arrangement * (smtk::model::Resource::*)(::smtk::common::UUID const &, ::smtk::model::ArrangementKind, int)) &smtk::model::Resource::findArrangement, py::arg("entityId"), py::arg("kind"), py::arg("index"))
    .def("findArrangementInvolvingEntity", &smtk::model::Resource::findArrangementInvolvingEntity, py::arg("entityId"), py::arg("kind"), py::arg("involved"))
    .def("findCellHasUseWithSense", &smtk::model::Resource::findCellHasUseWithSense, py::arg("cellId"), py::arg("use"), py::arg("sense"))
    .def("findCellHasUsesWithOrientation", &smtk::model::Resource::findCellHasUsesWithOrientation, py::arg("cellId"), py::arg("orient"))
    .def("findCreateOrReplaceCellUseOfSenseAndOrientation", &smtk::model::Resource::findCreateOrReplaceCellUseOfSenseAndOrientation, py::arg("cell"), py::arg("sense"), py::arg("o"), py::arg("replacement") = smtk::common::UUID::null())
    .def("findDualArrangements", &smtk::model::Resource::findDualArrangements, py::arg("entityId"), py::arg("kind"), py::arg("index"), py::arg("duals"))
    .def("findEntitiesByProperty", (smtk::model::EntityRefArray (smtk::model::Resource::*)(::std::string const &, ::smtk::model::Integer)) &smtk::model::Resource::findEntitiesByProperty, py::arg("pname"), py::arg("pval"))
    .def("findEntitiesByProperty", (smtk::model::EntityRefArray (smtk::model::Resource::*)(::std::string const &, ::smtk::model::Float)) &smtk::model::Resource::findEntitiesByProperty, py::arg("pname"), py::arg("pval"))
    .def("findEntitiesByProperty", (smtk::model::EntityRefArray (smtk::model::Resource::*)(::std::string const &, ::std::string const &)) &smtk::model::Resource::findEntitiesByProperty, py::arg("pname"), py::arg("pval"))
    .def("findEntitiesByProperty", (smtk::model::EntityRefArray (smtk::model::Resource::*)(::std::string const &, ::smtk::model::IntegerList const &)) &smtk::model::Resource::findEntitiesByProperty, py::arg("pname"), py::arg("pval"))
    .def("findEntitiesByProperty", (smtk::model::EntityRefArray (smtk::model::Resource::*)(::std::string const &, ::smtk::model::FloatList const &)) &smtk::model::Resource::findEntitiesByProperty, py::arg("pname"), py::arg("pval"))
    .def("findEntitiesByProperty", (smtk::model::EntityRefArray (smtk::model::Resource::*)(::std::string const &, ::smtk::model::StringList const &)) &smtk::model::Resource::findEntitiesByProperty, py::arg("pname"), py::arg("pval"))
    .def("_findEntitiesOfType", &smtk::model::Resource::findEntitiesOfType, py::arg("flags"), py::arg("exactMatch") = true)
    .def("findEntity", &smtk::model::Resource::findEntity, py::arg("uid"), py::arg("trySessions") = true)
    .def("findOrAddEntityToGroup", &smtk::model::Resource::findOrAddEntityToGroup, py::arg("grp"), py::arg("ent"))
    .def("findOrAddIncludedShell", &smtk::model::Resource::findOrAddIncludedShell, py::arg("parentUseOrShell"), py::arg("shellToInclude"))
    .def("findOrAddInclusionToCellOrModel", &smtk::model::Resource::findOrAddInclusionToCellOrModel, py::arg("cell"), py::arg("inclusion"))
    .def("findOrAddUseToShell", &smtk::model::Resource::findOrAddUseToShell, py::arg("shell"), py::arg("use"))
    .def("floatProperty", (smtk::model::FloatList const & (smtk::model::Resource::*)(::smtk::common::UUID const &, ::std::string const &) const) &smtk::model::Resource::floatProperty, py::arg("entity"), py::arg("propName"))
    .def("floatProperty", (smtk::model::FloatList & (smtk::model::Resource::*)(::smtk::common::UUID const &, ::std::string const &)) &smtk::model::Resource::floatProperty, py::arg("entity"), py::arg("propName"))
    .def("hardErase", &smtk::model::Resource::hardErase, py::arg("eref"), py::arg("flags") = ::smtk::model::SessionInfoBits(::smtk::model::SessionInformation::SESSION_EVERYTHING))
    .def("hasArrangementsOfKindForEntity", (smtk::model::Arrangements const * (smtk::model::Resource::*)(::smtk::common::UUID const &, ::smtk::model::ArrangementKind) const) &smtk::model::Resource::hasArrangementsOfKindForEntity, py::arg("cellId"), py::arg("arg1"))
    .def("hasArrangementsOfKindForEntity", (smtk::model::Arrangements * (smtk::model::Resource::*)(::smtk::common::UUID const &, ::smtk::model::ArrangementKind)) &smtk::model::Resource::hasArrangementsOfKindForEntity, py::arg("cellId"), py::arg("arg1"))
    .def("hasAttribute", &smtk::model::Resource::hasAttribute, py::arg("attribId"), py::arg("toEntity"))
    .def("hasFloatProperty", &smtk::model::Resource::hasFloatProperty, py::arg("entity"), py::arg("propName"))
    .def("hasIntegerProperty", &smtk::model::Resource::hasIntegerProperty, py::arg("entity"), py::arg("propName"))
    .def("hasStringProperty", &smtk::model::Resource::hasStringProperty, py::arg("entity"), py::arg("propName"))
    .def("higherDimensionalBordants", &smtk::model::Resource::higherDimensionalBordants, py::arg("ofEntity"), py::arg("higherDimension"))
    .def("insertAuxiliaryGeometry", &smtk::model::Resource::insertAuxiliaryGeometry, py::arg("uid"), py::arg("dim") = -1)
    .def("insertCellOfDimension", &smtk::model::Resource::insertCellOfDimension, py::arg("dim"))
    .def("insertChain", &smtk::model::Resource::insertChain, py::arg("uid"))
    .def("insertEdge", &smtk::model::Resource::insertEdge, py::arg("uid"))
    .def("insertEdgeUse", &smtk::model::Resource::insertEdgeUse, py::arg("uid"))
    .def("insertEntity", &smtk::model::Resource::insertEntity, py::arg("cell"))
    .def("insertEntityOfTypeAndDimension", &smtk::model::Resource::insertEntityOfTypeAndDimension, py::arg("entityFlags"), py::arg("dim"))
    .def("insertEntityReferences", &smtk::model::Resource::insertEntityReferences, py::arg("c"))
    .def("insertFace", &smtk::model::Resource::insertFace, py::arg("uid"))
    .def("insertFaceUse", &smtk::model::Resource::insertFaceUse, py::arg("uid"))
    .def("insertGroup", &smtk::model::Resource::insertGroup, py::arg("uid"), py::arg("extraFlags") = 0, py::arg("name") = std::string())
    .def("insertLoop", &smtk::model::Resource::insertLoop, py::arg("uid"))
    .def("insertModel", &smtk::model::Resource::insertModel, py::arg("uid"), py::arg("parametricDim") = 3, py::arg("embeddingDim") = 3, py::arg("name") = std::string())
    .def("insertShell", &smtk::model::Resource::insertShell, py::arg("uid"))
    .def("insertVertex", &smtk::model::Resource::insertVertex, py::arg("uid"))
    .def("insertVertexUse", &smtk::model::Resource::insertVertexUse, py::arg("uid"))
    .def("insertVolume", &smtk::model::Resource::insertVolume, py::arg("uid"))
    .def("insertVolumeUse", &smtk::model::Resource::insertVolumeUse, py::arg("uid"))
    .def("integerProperty", (smtk::model::IntegerList const & (smtk::model::Resource::*)(::smtk::common::UUID const &, ::std::string const &) const) &smtk::model::Resource::integerProperty, py::arg("entity"), py::arg("propName"))
    .def("integerProperty", (smtk::model::IntegerList & (smtk::model::Resource::*)(::smtk::common::UUID const &, ::std::string const &)) &smtk::model::Resource::integerProperty, py::arg("entity"), py::arg("propName"))
    .def("log", &smtk::model::Resource::log)
    .def("lowerDimensionalBoundaries", &smtk::model::Resource::lowerDimensionalBoundaries, py::arg("ofEntity"), py::arg("lowerDimension"))
    .def("meshTessellations", &smtk::model::Resource::meshTessellations)
    .def("modelOwningEntity", &smtk::model::Resource::modelOwningEntity, py::arg("uid"))
    .def("name", (std::string (smtk::model::Resource::*)() const) &smtk::resource::Resource::name)
    .def("name", (std::string (smtk::model::Resource::*)(const smtk::common::UUID& ) const) &smtk::model::Resource::name, py::arg("ofEntity"))
    .def("observe", (void (smtk::model::Resource::*)(::smtk::model::ResourceEventType, ::smtk::model::ConditionCallback, void *)) &smtk::model::Resource::observe, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("observe", (void (smtk::model::Resource::*)(::smtk::model::ResourceEventType, ::smtk::model::OneToOneCallback, void *)) &smtk::model::Resource::observe, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("observe", (void (smtk::model::Resource::*)(::smtk::model::ResourceEventType, ::smtk::model::OneToManyCallback, void *)) &smtk::model::Resource::observe, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("registerSession", &smtk::model::Resource::registerSession, py::arg("session"))
    .def("removeEntityReferences", &smtk::model::Resource::removeEntityReferences, py::arg("c"))
    .def("removeFloatProperty", &smtk::model::Resource::removeFloatProperty, py::arg("entity"), py::arg("propName"))
    .def("removeIntegerProperty", &smtk::model::Resource::removeIntegerProperty, py::arg("entity"), py::arg("propName"))
    .def("removeStringProperty", &smtk::model::Resource::removeStringProperty, py::arg("entity"), py::arg("propName"))
    .def("removeTessellation", &smtk::model::Resource::removeTessellation, py::arg("cellId"), py::arg("removeGen") = false)
    .def("sessionData", &smtk::model::Resource::sessionData, py::arg("sessRef"))
    .def("sessionOwningEntity", &smtk::model::Resource::sessionOwningEntity, py::arg("uid"))
    .def("sessions", &smtk::model::Resource::sessions)
    .def("setBoundingBox", &smtk::model::Resource::setBoundingBox, py::arg("cellId"), py::arg("coords"), py::arg("providedBBox") = 0)
    .def("setCellOfDimension", &smtk::model::Resource::setCellOfDimension, py::arg("uid"), py::arg("dim"))
    .def("setChain", (smtk::model::Chain (smtk::model::Resource::*)(::smtk::common::UUID const &, ::smtk::model::EdgeUse const &)) &smtk::model::Resource::setChain, py::arg("uid"), py::arg("use"))
    .def("setChain", (smtk::model::Chain (smtk::model::Resource::*)(::smtk::common::UUID const &, ::smtk::model::Chain const &)) &smtk::model::Resource::setChain, py::arg("uid"), py::arg("parent"))
    .def("setEdgeUse", &smtk::model::Resource::setEdgeUse, py::arg("uid"), py::arg("src"), py::arg("sense"), py::arg("o"))
    .def("setEntity", &smtk::model::Resource::setEntity, py::arg("cell"))
    .def("setEntityOfTypeAndDimension", &smtk::model::Resource::setEntityOfTypeAndDimension, py::arg("uid"), py::arg("entityFlags"), py::arg("dim"))
    .def("setFaceUse", &smtk::model::Resource::setFaceUse, py::arg("uid"), py::arg("src"), py::arg("sense"), py::arg("o"))
    .def("setFloatProperty", (void (smtk::model::Resource::*)(::smtk::common::UUID const &, ::std::string const &, ::smtk::model::Float)) &smtk::model::Resource::setFloatProperty, py::arg("entity"), py::arg("propName"), py::arg("propValue"))
    .def("setFloatProperty", (void (smtk::model::Resource::*)(::smtk::common::UUID const &, ::std::string const &, ::smtk::model::FloatList const &)) &smtk::model::Resource::setFloatProperty, py::arg("entity"), py::arg("propName"), py::arg("propValue"))
    .def("setIntegerProperty", (void (smtk::model::Resource::*)(::smtk::common::UUID const &, ::std::string const &, ::smtk::model::Integer)) &smtk::model::Resource::setIntegerProperty, py::arg("entity"), py::arg("propName"), py::arg("propValue"))
    .def("setIntegerProperty", (void (smtk::model::Resource::*)(::smtk::common::UUID const &, ::std::string const &, ::smtk::model::IntegerList const &)) &smtk::model::Resource::setIntegerProperty, py::arg("entity"), py::arg("propName"), py::arg("propValue"))
    .def("setLoop", (smtk::model::Loop (smtk::model::Resource::*)(::smtk::common::UUID const &, ::smtk::model::FaceUse const &)) &smtk::model::Resource::setLoop, py::arg("uid"), py::arg("use"))
    .def("setLoop", (smtk::model::Loop (smtk::model::Resource::*)(::smtk::common::UUID const &, ::smtk::model::Loop const &)) &smtk::model::Resource::setLoop, py::arg("uid"), py::arg("parent"))
    .def("setMeshTessellations", &smtk::model::Resource::setMeshTessellations)
    .def("setShell", (smtk::model::Shell (smtk::model::Resource::*)(::smtk::common::UUID const &, ::smtk::model::VolumeUse const &)) &smtk::model::Resource::setShell, py::arg("uid"), py::arg("use"))
    .def("setShell", (smtk::model::Shell (smtk::model::Resource::*)(::smtk::common::UUID const &, ::smtk::model::Shell const &)) &smtk::model::Resource::setShell, py::arg("uid"), py::arg("parent"))
    .def("setStringProperty", (void (smtk::model::Resource::*)(::smtk::common::UUID const &, ::std::string const &, ::smtk::model::String const &)) &smtk::model::Resource::setStringProperty, py::arg("entity"), py::arg("propName"), py::arg("propValue"))
    .def("setStringProperty", (void (smtk::model::Resource::*)(::smtk::common::UUID const &, ::std::string const &, ::smtk::model::StringList const &)) &smtk::model::Resource::setStringProperty, py::arg("entity"), py::arg("propName"), py::arg("propValue"))
    .def("setTessellation", &smtk::model::Resource::setTessellationAndBoundingBox, py::arg("cellId"), py::arg("geom"), py::arg("analysis") = 0, py::arg("gen") = nullptr)
    .def("setTessellationAndBoundingBox", &smtk::model::Resource::setTessellationAndBoundingBox, py::arg("cellId"), py::arg("geom"), py::arg("analysis") = 0, py::arg("gen") = nullptr)
    .def("setBoundingBox", &smtk::model::Resource::setBoundingBox, py::arg("cellId"), py::arg("coords"), py::arg("providedBBox") = 0)
    .def("setVertexUse", &smtk::model::Resource::setVertexUse, py::arg("uid"), py::arg("src"), py::arg("sense"))
    .def("setVolumeUse", &smtk::model::Resource::setVolumeUse, py::arg("uid"), py::arg("src"))
    .def_static("shortUUIDName", &smtk::model::Resource::shortUUIDName, py::arg("uid"), py::arg("entityFlags"))
    .def("stringProperty", (smtk::model::StringList const & (smtk::model::Resource::*)(::smtk::common::UUID const &, ::std::string const &) const) &smtk::model::Resource::stringProperty, py::arg("entity"), py::arg("propName"))
    .def("stringProperty", (smtk::model::StringList & (smtk::model::Resource::*)(::smtk::common::UUID const &, ::std::string const &)) &smtk::model::Resource::stringProperty, py::arg("entity"), py::arg("propName"))
    .def("tessellations", (smtk::model::UUIDsToTessellations & (smtk::model::Resource::*)()) &smtk::model::Resource::tessellations)
    .def("tessellations", (smtk::model::UUIDsToTessellations const & (smtk::model::Resource::*)() const) &smtk::model::Resource::tessellations)
    .def("topology", (smtk::model::UUIDsToEntities & (smtk::model::Resource::*)()) &smtk::model::Resource::topology)
    .def("topology", (smtk::model::UUIDsToEntities const & (smtk::model::Resource::*)() const) &smtk::model::Resource::topology)
    .def("trigger", (void (smtk::model::Resource::*)(::smtk::model::ResourceEventType, ::smtk::model::EntityRef const &)) &smtk::model::Resource::trigger, py::arg("event"), py::arg("src"))
    .def("trigger", (void (smtk::model::Resource::*)(::smtk::model::ResourceEventType, ::smtk::model::EntityRef const &, ::smtk::model::EntityRef const &)) &smtk::model::Resource::trigger, py::arg("event"), py::arg("src"), py::arg("related"))
    .def("trigger", (void (smtk::model::Resource::*)(::smtk::model::ResourceEventType, ::smtk::model::EntityRef const &, ::smtk::model::EntityRefArray const &)) &smtk::model::Resource::trigger, py::arg("event"), py::arg("src"), py::arg("related"))
    .def("type", (smtk::model::BitFlags (smtk::model::Resource::*)(const ::smtk::common::UUID&) const) &smtk::model::Resource::type, py::arg("ofEntity"))
    .def("unarrangeEntity", &smtk::model::Resource::unarrangeEntity, py::arg("entityId"), py::arg("arg1"), py::arg("index"), py::arg("removeIfLast") = false)
    .def("unobserve", (void (smtk::model::Resource::*)(::smtk::model::ResourceEventType, ::smtk::model::ConditionCallback, void *)) &smtk::model::Resource::unobserve, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("unobserve", (void (smtk::model::Resource::*)(::smtk::model::ResourceEventType, ::smtk::model::OneToOneCallback, void *)) &smtk::model::Resource::unobserve, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("unobserve", (void (smtk::model::Resource::*)(::smtk::model::ResourceEventType, ::smtk::model::OneToManyCallback, void *)) &smtk::model::Resource::unobserve, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("unregisterSession", &smtk::model::Resource::unregisterSession, py::arg("session"), py::arg("expungeSession") = true)
    .def("unusedUUID", &smtk::model::Resource::unusedUUID)
    .def("useOrShellIncludesShells", &smtk::model::Resource::useOrShellIncludesShells, py::arg("cellUseOrShell"))
    .def_static("CastTo", [](const std::shared_ptr<smtk::resource::Resource> i) {
        return std::dynamic_pointer_cast<smtk::model::Resource>(i);
      })
    ;
  return instance;
}

#endif
