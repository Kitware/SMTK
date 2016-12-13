//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Manager_h
#define pybind_smtk_model_Manager_h

#include <pybind11/pybind11.h>

#include "smtk/model/Manager.h"

#include "smtk/attribute/System.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/io/Logger.h"
#include "smtk/mesh/Manager.h"
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
#include "smtk/model/Operator.h"
#include "smtk/model/Session.h"
#include "smtk/model/SessionRef.h"
#include "smtk/model/Shell.h"
#include "smtk/model/StringData.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/VertexUse.h"
#include "smtk/model/Volume.h"
#include "smtk/model/VolumeUse.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::Manager > pybind11_init_smtk_model_Manager(py::module &m)
{
  PySharedPtrClass< smtk::model::Manager > instance(m, "Manager", py::dynamic_attr());
  instance
    .def(py::init<::smtk::model::Manager const &>())
    .def(py::init<>())
    .def(py::init<::std::shared_ptr<std::map<smtk::common::UUID, smtk::model::Entity, std::less<smtk::common::UUID>, std::allocator<std::pair<const smtk::common::UUID, smtk::model::Entity> > > >, ::std::shared_ptr<std::map<smtk::common::UUID, std::map<smtk::model::ArrangementKind, std::vector<smtk::model::Arrangement, std::allocator<smtk::model::Arrangement> >, std::less<smtk::model::ArrangementKind>, std::allocator<std::pair<const smtk::model::ArrangementKind, std::vector<smtk::model::Arrangement, std::allocator<smtk::model::Arrangement> > > > >, std::less<smtk::common::UUID>, std::allocator<std::pair<const smtk::common::UUID, std::map<smtk::model::ArrangementKind, std::vector<smtk::model::Arrangement, std::allocator<smtk::model::Arrangement> >, std::less<smtk::model::ArrangementKind>, std::allocator<std::pair<const smtk::model::ArrangementKind, std::vector<smtk::model::Arrangement, std::allocator<smtk::model::Arrangement> > > > > > > > >, ::std::shared_ptr<std::map<smtk::common::UUID, smtk::model::Tessellation, std::less<smtk::common::UUID>, std::allocator<std::pair<const smtk::common::UUID, smtk::model::Tessellation> > > >, ::std::shared_ptr<std::map<smtk::common::UUID, smtk::model::Tessellation, std::less<smtk::common::UUID>, std::allocator<std::pair<const smtk::common::UUID, smtk::model::Tessellation> > > >, ::std::shared_ptr<smtk::mesh::Manager>, ::std::shared_ptr<std::map<smtk::common::UUID, smtk::model::AttributeAssignments, std::less<smtk::common::UUID>, std::allocator<std::pair<const smtk::common::UUID, smtk::model::AttributeAssignments> > > > >())
    .def("deepcopy", (smtk::model::Manager & (smtk::model::Manager::*)(::smtk::model::Manager const &)) &smtk::model::Manager::operator=)
    .def("addAuxiliaryGeometry", (smtk::model::AuxiliaryGeometry (smtk::model::Manager::*)(int)) &smtk::model::Manager::addAuxiliaryGeometry, py::arg("dim") = -1)
    .def("addAuxiliaryGeometry", (smtk::model::AuxiliaryGeometry (smtk::model::Manager::*)(::smtk::model::Model const &, int)) &smtk::model::Manager::addAuxiliaryGeometry, py::arg("parent"), py::arg("dim") = -1)
    .def("addAuxiliaryGeometry", (smtk::model::AuxiliaryGeometry (smtk::model::Manager::*)(::smtk::model::AuxiliaryGeometry const &, int)) &smtk::model::Manager::addAuxiliaryGeometry, py::arg("parent"), py::arg("dim") = -1)
    .def("addCellOfDimension", &smtk::model::Manager::addCellOfDimension, py::arg("dim"))
    .def("addCellOfDimensionWithUUID", &smtk::model::Manager::addCellOfDimensionWithUUID, py::arg("uid"), py::arg("dim"))
    .def("addChain", (smtk::model::Chain (smtk::model::Manager::*)()) &smtk::model::Manager::addChain)
    .def("addChain", (smtk::model::Chain (smtk::model::Manager::*)(::smtk::model::EdgeUse const &)) &smtk::model::Manager::addChain, py::arg("arg0"))
    .def("addChain", (smtk::model::Chain (smtk::model::Manager::*)(::smtk::model::Chain const &)) &smtk::model::Manager::addChain, py::arg("arg0"))
    .def("addDualArrangement", &smtk::model::Manager::addDualArrangement, py::arg("parent"), py::arg("child"), py::arg("kind"), py::arg("sense"), py::arg("orientation"))
    .def("addEdge", &smtk::model::Manager::addEdge)
    .def("addEdgeUse", (smtk::model::EdgeUse (smtk::model::Manager::*)()) &smtk::model::Manager::addEdgeUse)
    .def("addEdgeUse", (smtk::model::EdgeUse (smtk::model::Manager::*)(::smtk::model::Edge const &, int, ::smtk::model::Orientation)) &smtk::model::Manager::addEdgeUse, py::arg("src"), py::arg("sense"), py::arg("o"))
    .def("addEntity", &smtk::model::Manager::addEntity, py::arg("cell"))
    .def("addEntityOfTypeAndDimension", &smtk::model::Manager::addEntityOfTypeAndDimension, py::arg("entityFlags"), py::arg("dim"))
    .def("addEntityOfTypeAndDimensionWithUUID", &smtk::model::Manager::addEntityOfTypeAndDimensionWithUUID, py::arg("uid"), py::arg("entityFlags"), py::arg("dim"))
    .def("addEntityWithUUID", &smtk::model::Manager::addEntityWithUUID, py::arg("uid"), py::arg("cell"))
    .def("addFace", &smtk::model::Manager::addFace)
    .def("addFaceUse", (smtk::model::FaceUse (smtk::model::Manager::*)()) &smtk::model::Manager::addFaceUse)
    .def("addFaceUse", (smtk::model::FaceUse (smtk::model::Manager::*)(::smtk::model::Face const &, int, ::smtk::model::Orientation)) &smtk::model::Manager::addFaceUse, py::arg("src"), py::arg("sense"), py::arg("o"))
    .def("addGroup", &smtk::model::Manager::addGroup, py::arg("extraFlags") = 0, py::arg("name") = std::string())
    .def("addInstance", (smtk::model::Instance (smtk::model::Manager::*)()) &smtk::model::Manager::addInstance)
    .def("addInstance", (smtk::model::Instance (smtk::model::Manager::*)(::smtk::model::EntityRef const &)) &smtk::model::Manager::addInstance, py::arg("instanceOf"))
    .def("addLoop", (smtk::model::Loop (smtk::model::Manager::*)()) &smtk::model::Manager::addLoop)
    .def("addLoop", (smtk::model::Loop (smtk::model::Manager::*)(::smtk::model::FaceUse const &)) &smtk::model::Manager::addLoop, py::arg("arg0"))
    .def("addLoop", (smtk::model::Loop (smtk::model::Manager::*)(::smtk::model::Loop const &)) &smtk::model::Manager::addLoop, py::arg("arg0"))
    .def("addModel", &smtk::model::Manager::addModel, py::arg("parametricDim") = 3, py::arg("embeddingDim") = 3, py::arg("name") = std::string())
    .def("addShell", (smtk::model::Shell (smtk::model::Manager::*)()) &smtk::model::Manager::addShell)
    .def("addShell", (smtk::model::Shell (smtk::model::Manager::*)(::smtk::model::Volume const &)) &smtk::model::Manager::addShell, py::arg("src"))
    .def("addShell", (smtk::model::Shell (smtk::model::Manager::*)(::smtk::model::VolumeUse const &)) &smtk::model::Manager::addShell, py::arg("src"))
    .def("addToGroup", &smtk::model::Manager::addToGroup, py::arg("groupId"), py::arg("uids"))
    .def("addVertex", &smtk::model::Manager::addVertex)
    .def("addVertexUse", (smtk::model::VertexUse (smtk::model::Manager::*)()) &smtk::model::Manager::addVertexUse)
    .def("addVertexUse", (smtk::model::VertexUse (smtk::model::Manager::*)(::smtk::model::Vertex const &, int)) &smtk::model::Manager::addVertexUse, py::arg("src"), py::arg("sense"))
    .def("addVolume", &smtk::model::Manager::addVolume)
    .def("addVolumeUse", (smtk::model::VolumeUse (smtk::model::Manager::*)()) &smtk::model::Manager::addVolumeUse)
    .def("addVolumeUse", (smtk::model::VolumeUse (smtk::model::Manager::*)(::smtk::model::Volume const &)) &smtk::model::Manager::addVolumeUse, py::arg("src"))
    .def("adjacentEntities", &smtk::model::Manager::adjacentEntities, py::arg("ofEntity"), py::arg("ofDimension"))
    .def("analysisMesh", (smtk::model::UUIDsToTessellations & (smtk::model::Manager::*)()) &smtk::model::Manager::analysisMesh)
    .def("analysisMesh", (smtk::model::UUIDsToTessellations const & (smtk::model::Manager::*)() const) &smtk::model::Manager::analysisMesh)
    .def("arrangeEntity", &smtk::model::Manager::arrangeEntity, py::arg("entityId"), py::arg("arg1"), py::arg("arr"), py::arg("index") = -1)
    .def("arrangements", (smtk::model::UUIDsToArrangements & (smtk::model::Manager::*)()) &smtk::model::Manager::arrangements)
    .def("arrangements", (smtk::model::UUIDsToArrangements const & (smtk::model::Manager::*)() const) &smtk::model::Manager::arrangements)
    .def("arrangementsOfKindForEntity", &smtk::model::Manager::arrangementsOfKindForEntity, py::arg("cellId"), py::arg("arg1"))
    .def("assignDefaultName", (std::string (smtk::model::Manager::*)(::smtk::common::UUID const &)) &smtk::model::Manager::assignDefaultName, py::arg("uid"))
    .def("assignDefaultNameIfMissing", &smtk::model::Manager::assignDefaultNameIfMissing, py::arg("uid"))
    .def("assignDefaultNames", &smtk::model::Manager::assignDefaultNames)
    .def("assignDefaultNamesToModelChildren", &smtk::model::Manager::assignDefaultNamesToModelChildren, py::arg("modelId"))
    .def("associateAttribute", &smtk::model::Manager::associateAttribute, py::arg("sys"), py::arg("attribId"), py::arg("toEntity"))
    .def("attributeAssignments", &smtk::model::Manager::attributeAssignments)
    .def("bordantEntities", (smtk::common::UUIDs (smtk::model::Manager::*)(::smtk::common::UUID const &, int) const) &smtk::model::Manager::bordantEntities, py::arg("ofEntity"), py::arg("ofDimension") = -2)
    .def("bordantEntities", (smtk::common::UUIDs (smtk::model::Manager::*)(::smtk::common::UUIDs const &, int) const) &smtk::model::Manager::bordantEntities, py::arg("ofEntities"), py::arg("ofDimension") = -2)
    .def("boundaryEntities", (smtk::common::UUIDs (smtk::model::Manager::*)(::smtk::common::UUID const &, int) const) &smtk::model::Manager::boundaryEntities, py::arg("ofEntity"), py::arg("ofDimension") = -2)
    .def("boundaryEntities", (smtk::common::UUIDs (smtk::model::Manager::*)(::smtk::common::UUIDs const &, int) const) &smtk::model::Manager::boundaryEntities, py::arg("ofEntities"), py::arg("ofDimension") = -2)
    .def("cellHasUseOfSenseAndOrientation", &smtk::model::Manager::cellHasUseOfSenseAndOrientation, py::arg("cell"), py::arg("sense"), py::arg("o"))
    .def("classname", &smtk::model::Manager::classname)
    .def("clearArrangements", &smtk::model::Manager::clearArrangements, py::arg("entityId"))
    .def("closeSession", &smtk::model::Manager::closeSession, py::arg("sess"))
    .def_static("create", (std::shared_ptr<smtk::model::Manager> (*)()) &smtk::model::Manager::create)
    .def_static("create", (std::shared_ptr<smtk::model::Manager> (*)(::std::shared_ptr<smtk::model::Manager> &)) &smtk::model::Manager::create, py::arg("ref"))
    .def("createIncludedShell", &smtk::model::Manager::createIncludedShell, py::arg("cellUseOrShell"))
    .def("createSession", (smtk::model::SessionRef (smtk::model::Manager::*)(::std::string const &)) &smtk::model::Manager::createSession, py::arg("sname"))
    .def("createSession", (smtk::model::SessionRef (smtk::model::Manager::*)(::std::string const &, ::smtk::model::SessionRef const &)) &smtk::model::Manager::createSession, py::arg("sname"), py::arg("sessionIdSpecifier"))
    .def_static("createSessionOfType", &smtk::model::Manager::createSessionOfType, py::arg("sname"))
    .def("dimension", &smtk::model::Manager::dimension, py::arg("ofEntity"))
    .def("disassociateAttribute", &smtk::model::Manager::disassociateAttribute, py::arg("sys"), py::arg("attribId"), py::arg("fromEntity"), py::arg("reverse") = true)
    .def("elideEntityReferences", &smtk::model::Manager::elideEntityReferences, py::arg("c"))
    .def("elideOneEntityReference", &smtk::model::Manager::elideOneEntityReference, py::arg("c"), py::arg("r"))
    .def("entitiesMatchingFlags", &smtk::model::Manager::entitiesMatchingFlags, py::arg("mask"), py::arg("exactMatch") = true)
    .def("entitiesOfDimension", &smtk::model::Manager::entitiesOfDimension, py::arg("dim"))
    .def("erase", (smtk::model::SessionInfoBits (smtk::model::Manager::*)(::smtk::common::UUID const &, ::smtk::model::SessionInfoBits)) &smtk::model::Manager::erase, py::arg("uid"), py::arg("flags") = ::smtk::model::SessionInfoBits(::smtk::model::SessionInformation::SESSION_EVERYTHING))
    .def("erase", (smtk::model::SessionInfoBits (smtk::model::Manager::*)(::smtk::model::EntityRef const &, ::smtk::model::SessionInfoBits)) &smtk::model::Manager::erase, py::arg("entityref"), py::arg("flags") = ::smtk::model::SessionInfoBits(::smtk::model::SessionInformation::SESSION_EVERYTHING))
    .def("eraseModel", &smtk::model::Manager::eraseModel, py::arg("entityref"), py::arg("flags") = ::smtk::model::SessionInfoBits(::smtk::model::SessionInformation::SESSION_EVERYTHING))
    .def("findArrangement", (smtk::model::Arrangement const * (smtk::model::Manager::*)(::smtk::common::UUID const &, ::smtk::model::ArrangementKind, int) const) &smtk::model::Manager::findArrangement, py::arg("entityId"), py::arg("kind"), py::arg("index"))
    .def("findArrangement", (smtk::model::Arrangement * (smtk::model::Manager::*)(::smtk::common::UUID const &, ::smtk::model::ArrangementKind, int)) &smtk::model::Manager::findArrangement, py::arg("entityId"), py::arg("kind"), py::arg("index"))
    .def("findArrangementInvolvingEntity", &smtk::model::Manager::findArrangementInvolvingEntity, py::arg("entityId"), py::arg("kind"), py::arg("involved"))
    .def("findCellHasUseWithSense", &smtk::model::Manager::findCellHasUseWithSense, py::arg("cellId"), py::arg("use"), py::arg("sense"))
    .def("findCellHasUsesWithOrientation", &smtk::model::Manager::findCellHasUsesWithOrientation, py::arg("cellId"), py::arg("orient"))
    .def("findCreateOrReplaceCellUseOfSenseAndOrientation", &smtk::model::Manager::findCreateOrReplaceCellUseOfSenseAndOrientation, py::arg("cell"), py::arg("sense"), py::arg("o"), py::arg("replacement") = smtk::common::UUID::null())
    .def("findDualArrangements", &smtk::model::Manager::findDualArrangements, py::arg("entityId"), py::arg("kind"), py::arg("index"), py::arg("duals"))
    .def("findEntitiesByProperty", (smtk::model::EntityRefArray (smtk::model::Manager::*)(::std::string const &, ::smtk::model::Integer)) &smtk::model::Manager::findEntitiesByProperty, py::arg("pname"), py::arg("pval"))
    .def("findEntitiesByProperty", (smtk::model::EntityRefArray (smtk::model::Manager::*)(::std::string const &, ::smtk::model::Float)) &smtk::model::Manager::findEntitiesByProperty, py::arg("pname"), py::arg("pval"))
    .def("findEntitiesByProperty", (smtk::model::EntityRefArray (smtk::model::Manager::*)(::std::string const &, ::std::string const &)) &smtk::model::Manager::findEntitiesByProperty, py::arg("pname"), py::arg("pval"))
    .def("findEntitiesByProperty", (smtk::model::EntityRefArray (smtk::model::Manager::*)(::std::string const &, ::smtk::model::IntegerList const &)) &smtk::model::Manager::findEntitiesByProperty, py::arg("pname"), py::arg("pval"))
    .def("findEntitiesByProperty", (smtk::model::EntityRefArray (smtk::model::Manager::*)(::std::string const &, ::smtk::model::FloatList const &)) &smtk::model::Manager::findEntitiesByProperty, py::arg("pname"), py::arg("pval"))
    .def("findEntitiesByProperty", (smtk::model::EntityRefArray (smtk::model::Manager::*)(::std::string const &, ::smtk::model::StringList const &)) &smtk::model::Manager::findEntitiesByProperty, py::arg("pname"), py::arg("pval"))
    .def("_findEntitiesOfType", &smtk::model::Manager::findEntitiesOfType, py::arg("flags"), py::arg("exactMatch") = true)
    .def("findEntity", (smtk::model::Entity const * (smtk::model::Manager::*)(::smtk::common::UUID const &, bool) const) &smtk::model::Manager::findEntity, py::arg("uid"), py::arg("trySessions") = true, py::return_value_policy::reference)
    .def("findEntity", (smtk::model::Entity * (smtk::model::Manager::*)(::smtk::common::UUID const &, bool)) &smtk::model::Manager::findEntity, py::arg("uid"), py::arg("trySessions") = true, py::return_value_policy::reference)
    .def("findOrAddEntityToGroup", &smtk::model::Manager::findOrAddEntityToGroup, py::arg("grp"), py::arg("ent"))
    .def("findOrAddIncludedShell", &smtk::model::Manager::findOrAddIncludedShell, py::arg("parentUseOrShell"), py::arg("shellToInclude"))
    .def("findOrAddInclusionToCellOrModel", &smtk::model::Manager::findOrAddInclusionToCellOrModel, py::arg("cell"), py::arg("inclusion"))
    .def("findOrAddUseToShell", &smtk::model::Manager::findOrAddUseToShell, py::arg("shell"), py::arg("use"))
    .def("floatProperties", (smtk::model::UUIDsToFloatData & (smtk::model::Manager::*)()) &smtk::model::Manager::floatProperties)
    .def("floatProperties", (smtk::model::UUIDsToFloatData const & (smtk::model::Manager::*)() const) &smtk::model::Manager::floatProperties)
    .def("floatPropertiesForEntity", (smtk::model::UUIDWithFloatProperties const (smtk::model::Manager::*)(::smtk::common::UUID const &) const) &smtk::model::Manager::floatPropertiesForEntity, py::arg("entity"))
    .def("floatPropertiesForEntity", (smtk::model::UUIDWithFloatProperties (smtk::model::Manager::*)(::smtk::common::UUID const &)) &smtk::model::Manager::floatPropertiesForEntity, py::arg("entity"))
    .def("floatProperty", (smtk::model::FloatList const & (smtk::model::Manager::*)(::smtk::common::UUID const &, ::std::string const &) const) &smtk::model::Manager::floatProperty, py::arg("entity"), py::arg("propName"))
    .def("floatProperty", (smtk::model::FloatList & (smtk::model::Manager::*)(::smtk::common::UUID const &, ::std::string const &)) &smtk::model::Manager::floatProperty, py::arg("entity"), py::arg("propName"))
    .def("hasArrangementsOfKindForEntity", (smtk::model::Arrangements const * (smtk::model::Manager::*)(::smtk::common::UUID const &, ::smtk::model::ArrangementKind) const) &smtk::model::Manager::hasArrangementsOfKindForEntity, py::arg("cellId"), py::arg("arg1"))
    .def("hasArrangementsOfKindForEntity", (smtk::model::Arrangements * (smtk::model::Manager::*)(::smtk::common::UUID const &, ::smtk::model::ArrangementKind)) &smtk::model::Manager::hasArrangementsOfKindForEntity, py::arg("cellId"), py::arg("arg1"))
    .def("hasAttribute", &smtk::model::Manager::hasAttribute, py::arg("attribId"), py::arg("toEntity"))
    .def("hasFloatProperty", &smtk::model::Manager::hasFloatProperty, py::arg("entity"), py::arg("propName"))
    .def("hasIntegerProperty", &smtk::model::Manager::hasIntegerProperty, py::arg("entity"), py::arg("propName"))
    .def("hasStringProperty", &smtk::model::Manager::hasStringProperty, py::arg("entity"), py::arg("propName"))
    .def("higherDimensionalBordants", &smtk::model::Manager::higherDimensionalBordants, py::arg("ofEntity"), py::arg("higherDimension"))
    .def("insertAuxiliaryGeometry", &smtk::model::Manager::insertAuxiliaryGeometry, py::arg("uid"), py::arg("dim") = -1)
    .def("insertCellOfDimension", &smtk::model::Manager::insertCellOfDimension, py::arg("dim"))
    .def("insertChain", &smtk::model::Manager::insertChain, py::arg("uid"))
    .def("insertEdge", &smtk::model::Manager::insertEdge, py::arg("uid"))
    .def("insertEdgeUse", &smtk::model::Manager::insertEdgeUse, py::arg("uid"))
    .def("insertEntity", &smtk::model::Manager::insertEntity, py::arg("cell"))
    .def("insertEntityOfTypeAndDimension", &smtk::model::Manager::insertEntityOfTypeAndDimension, py::arg("entityFlags"), py::arg("dim"))
    .def("insertEntityReferences", &smtk::model::Manager::insertEntityReferences, py::arg("c"))
    .def("insertFace", &smtk::model::Manager::insertFace, py::arg("uid"))
    .def("insertFaceUse", &smtk::model::Manager::insertFaceUse, py::arg("uid"))
    .def("insertGroup", &smtk::model::Manager::insertGroup, py::arg("uid"), py::arg("extraFlags") = 0, py::arg("name") = std::string())
    .def("insertLoop", &smtk::model::Manager::insertLoop, py::arg("uid"))
    .def("insertModel", &smtk::model::Manager::insertModel, py::arg("uid"), py::arg("parametricDim") = 3, py::arg("embeddingDim") = 3, py::arg("name") = std::string())
    .def("insertShell", &smtk::model::Manager::insertShell, py::arg("uid"))
    .def("insertVertex", &smtk::model::Manager::insertVertex, py::arg("uid"))
    .def("insertVertexUse", &smtk::model::Manager::insertVertexUse, py::arg("uid"))
    .def("insertVolume", &smtk::model::Manager::insertVolume, py::arg("uid"))
    .def("insertVolumeUse", &smtk::model::Manager::insertVolumeUse, py::arg("uid"))
    .def("integerProperties", (smtk::model::UUIDsToIntegerData & (smtk::model::Manager::*)()) &smtk::model::Manager::integerProperties)
    .def("integerProperties", (smtk::model::UUIDsToIntegerData const & (smtk::model::Manager::*)() const) &smtk::model::Manager::integerProperties)
    .def("integerPropertiesForEntity", (smtk::model::UUIDWithIntegerProperties const (smtk::model::Manager::*)(::smtk::common::UUID const &) const) &smtk::model::Manager::integerPropertiesForEntity, py::arg("entity"))
    .def("integerPropertiesForEntity", (smtk::model::UUIDWithIntegerProperties (smtk::model::Manager::*)(::smtk::common::UUID const &)) &smtk::model::Manager::integerPropertiesForEntity, py::arg("entity"))
    .def("integerProperty", (smtk::model::IntegerList const & (smtk::model::Manager::*)(::smtk::common::UUID const &, ::std::string const &) const) &smtk::model::Manager::integerProperty, py::arg("entity"), py::arg("propName"))
    .def("integerProperty", (smtk::model::IntegerList & (smtk::model::Manager::*)(::smtk::common::UUID const &, ::std::string const &)) &smtk::model::Manager::integerProperty, py::arg("entity"), py::arg("propName"))
    .def("log", &smtk::model::Manager::log)
    .def("lowerDimensionalBoundaries", &smtk::model::Manager::lowerDimensionalBoundaries, py::arg("ofEntity"), py::arg("lowerDimension"))
    .def("meshes", &smtk::model::Manager::meshes)
    .def("modelOwningEntity", &smtk::model::Manager::modelOwningEntity, py::arg("uid"))
    .def("name", &smtk::model::Manager::name, py::arg("ofEntity"))
    .def("observe", (void (smtk::model::Manager::*)(::smtk::model::ManagerEventType, ::smtk::model::ConditionCallback, void *)) &smtk::model::Manager::observe, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("observe", (void (smtk::model::Manager::*)(::smtk::model::ManagerEventType, ::smtk::model::OneToOneCallback, void *)) &smtk::model::Manager::observe, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("observe", (void (smtk::model::Manager::*)(::smtk::model::ManagerEventType, ::smtk::model::OneToManyCallback, void *)) &smtk::model::Manager::observe, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("observe", (void (smtk::model::Manager::*)(::smtk::model::OperatorEventType, ::smtk::model::BareOperatorCallback, void *)) &smtk::model::Manager::observe, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("registerSession", &smtk::model::Manager::registerSession, py::arg("session"))
    .def("removeEntityReferences", &smtk::model::Manager::removeEntityReferences, py::arg("c"))
    .def("removeFloatProperty", &smtk::model::Manager::removeFloatProperty, py::arg("entity"), py::arg("propName"))
    .def("removeIntegerProperty", &smtk::model::Manager::removeIntegerProperty, py::arg("entity"), py::arg("propName"))
    .def("removeStringProperty", &smtk::model::Manager::removeStringProperty, py::arg("entity"), py::arg("propName"))
    .def("removeTessellation", &smtk::model::Manager::removeTessellation, py::arg("cellId"), py::arg("removeGen") = false)
    .def("sessionData", &smtk::model::Manager::sessionData, py::arg("sessRef"))
    .def_static("sessionFileTypes", &smtk::model::Manager::sessionFileTypes, py::arg("sname"), py::arg("engine") = std::string())
    .def("sessionOwningEntity", &smtk::model::Manager::sessionOwningEntity, py::arg("uid"))
    .def_static("sessionTypeNames", &smtk::model::Manager::sessionTypeNames)
    .def("sessions", &smtk::model::Manager::sessions)
    .def("setCellOfDimension", &smtk::model::Manager::setCellOfDimension, py::arg("uid"), py::arg("dim"))
    .def("setChain", (smtk::model::Chain (smtk::model::Manager::*)(::smtk::common::UUID const &, ::smtk::model::EdgeUse const &)) &smtk::model::Manager::setChain, py::arg("uid"), py::arg("use"))
    .def("setChain", (smtk::model::Chain (smtk::model::Manager::*)(::smtk::common::UUID const &, ::smtk::model::Chain const &)) &smtk::model::Manager::setChain, py::arg("uid"), py::arg("parent"))
    .def("setEdgeUse", &smtk::model::Manager::setEdgeUse, py::arg("uid"), py::arg("src"), py::arg("sense"), py::arg("o"))
    .def("setEntity", &smtk::model::Manager::setEntity, py::arg("uid"), py::arg("cell"))
    .def("setEntityOfTypeAndDimension", &smtk::model::Manager::setEntityOfTypeAndDimension, py::arg("uid"), py::arg("entityFlags"), py::arg("dim"))
    .def("setFaceUse", &smtk::model::Manager::setFaceUse, py::arg("uid"), py::arg("src"), py::arg("sense"), py::arg("o"))
    .def("setFloatProperty", (void (smtk::model::Manager::*)(::smtk::common::UUID const &, ::std::string const &, ::smtk::model::Float)) &smtk::model::Manager::setFloatProperty, py::arg("entity"), py::arg("propName"), py::arg("propValue"))
    .def("setFloatProperty", (void (smtk::model::Manager::*)(::smtk::common::UUID const &, ::std::string const &, ::smtk::model::FloatList const &)) &smtk::model::Manager::setFloatProperty, py::arg("entity"), py::arg("propName"), py::arg("propValue"))
    .def("setIntegerProperty", (void (smtk::model::Manager::*)(::smtk::common::UUID const &, ::std::string const &, ::smtk::model::Integer)) &smtk::model::Manager::setIntegerProperty, py::arg("entity"), py::arg("propName"), py::arg("propValue"))
    .def("setIntegerProperty", (void (smtk::model::Manager::*)(::smtk::common::UUID const &, ::std::string const &, ::smtk::model::IntegerList const &)) &smtk::model::Manager::setIntegerProperty, py::arg("entity"), py::arg("propName"), py::arg("propValue"))
    .def("setLoop", (smtk::model::Loop (smtk::model::Manager::*)(::smtk::common::UUID const &, ::smtk::model::FaceUse const &)) &smtk::model::Manager::setLoop, py::arg("uid"), py::arg("use"))
    .def("setLoop", (smtk::model::Loop (smtk::model::Manager::*)(::smtk::common::UUID const &, ::smtk::model::Loop const &)) &smtk::model::Manager::setLoop, py::arg("uid"), py::arg("parent"))
    .def("setShell", (smtk::model::Shell (smtk::model::Manager::*)(::smtk::common::UUID const &, ::smtk::model::VolumeUse const &)) &smtk::model::Manager::setShell, py::arg("uid"), py::arg("use"))
    .def("setShell", (smtk::model::Shell (smtk::model::Manager::*)(::smtk::common::UUID const &, ::smtk::model::Shell const &)) &smtk::model::Manager::setShell, py::arg("uid"), py::arg("parent"))
    .def("setStringProperty", (void (smtk::model::Manager::*)(::smtk::common::UUID const &, ::std::string const &, ::smtk::model::String const &)) &smtk::model::Manager::setStringProperty, py::arg("entity"), py::arg("propName"), py::arg("propValue"))
    .def("setStringProperty", (void (smtk::model::Manager::*)(::smtk::common::UUID const &, ::std::string const &, ::smtk::model::StringList const &)) &smtk::model::Manager::setStringProperty, py::arg("entity"), py::arg("propName"), py::arg("propValue"))
    .def("setTessellation", &smtk::model::Manager::setTessellation, py::arg("cellId"), py::arg("geom"), py::arg("analysis") = 0, py::arg("gen") = nullptr)
    .def("setVertexUse", &smtk::model::Manager::setVertexUse, py::arg("uid"), py::arg("src"), py::arg("sense"))
    .def("setVolumeUse", &smtk::model::Manager::setVolumeUse, py::arg("uid"), py::arg("src"))
    .def_static("shortUUIDName", &smtk::model::Manager::shortUUIDName, py::arg("uid"), py::arg("entityFlags"))
    .def("stringProperties", (smtk::model::UUIDsToStringData & (smtk::model::Manager::*)()) &smtk::model::Manager::stringProperties)
    .def("stringProperties", (smtk::model::UUIDsToStringData const & (smtk::model::Manager::*)() const) &smtk::model::Manager::stringProperties)
    .def("stringPropertiesForEntity", (smtk::model::UUIDWithStringProperties const (smtk::model::Manager::*)(::smtk::common::UUID const &) const) &smtk::model::Manager::stringPropertiesForEntity, py::arg("entity"))
    .def("stringPropertiesForEntity", (smtk::model::UUIDWithStringProperties (smtk::model::Manager::*)(::smtk::common::UUID const &)) &smtk::model::Manager::stringPropertiesForEntity, py::arg("entity"))
    .def("stringProperty", (smtk::model::StringList const & (smtk::model::Manager::*)(::smtk::common::UUID const &, ::std::string const &) const) &smtk::model::Manager::stringProperty, py::arg("entity"), py::arg("propName"))
    .def("stringProperty", (smtk::model::StringList & (smtk::model::Manager::*)(::smtk::common::UUID const &, ::std::string const &)) &smtk::model::Manager::stringProperty, py::arg("entity"), py::arg("propName"))
    .def("tessellations", (smtk::model::UUIDsToTessellations & (smtk::model::Manager::*)()) &smtk::model::Manager::tessellations)
    .def("tessellations", (smtk::model::UUIDsToTessellations const & (smtk::model::Manager::*)() const) &smtk::model::Manager::tessellations)
    .def("topology", (smtk::model::UUIDsToEntities & (smtk::model::Manager::*)()) &smtk::model::Manager::topology)
    .def("topology", (smtk::model::UUIDsToEntities const & (smtk::model::Manager::*)() const) &smtk::model::Manager::topology)
    .def("trigger", (void (smtk::model::Manager::*)(::smtk::model::ManagerEventType, ::smtk::model::EntityRef const &)) &smtk::model::Manager::trigger, py::arg("event"), py::arg("src"))
    .def("trigger", (void (smtk::model::Manager::*)(::smtk::model::ManagerEventType, ::smtk::model::EntityRef const &, ::smtk::model::EntityRef const &)) &smtk::model::Manager::trigger, py::arg("event"), py::arg("src"), py::arg("related"))
    .def("trigger", (void (smtk::model::Manager::*)(::smtk::model::ManagerEventType, ::smtk::model::EntityRef const &, ::smtk::model::EntityRefArray const &)) &smtk::model::Manager::trigger, py::arg("event"), py::arg("src"), py::arg("related"))
    .def("trigger", (void (smtk::model::Manager::*)(::smtk::model::OperatorEventType, ::smtk::model::Operator const &)) &smtk::model::Manager::trigger, py::arg("event"), py::arg("src"))
    .def("type", &smtk::model::Manager::type, py::arg("ofEntity"))
    .def("unarrangeEntity", &smtk::model::Manager::unarrangeEntity, py::arg("entityId"), py::arg("arg1"), py::arg("index"), py::arg("removeIfLast") = false)
    .def("unobserve", (void (smtk::model::Manager::*)(::smtk::model::ManagerEventType, ::smtk::model::ConditionCallback, void *)) &smtk::model::Manager::unobserve, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("unobserve", (void (smtk::model::Manager::*)(::smtk::model::ManagerEventType, ::smtk::model::OneToOneCallback, void *)) &smtk::model::Manager::unobserve, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("unobserve", (void (smtk::model::Manager::*)(::smtk::model::ManagerEventType, ::smtk::model::OneToManyCallback, void *)) &smtk::model::Manager::unobserve, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("unobserve", (void (smtk::model::Manager::*)(::smtk::model::OperatorEventType, ::smtk::model::BareOperatorCallback, void *)) &smtk::model::Manager::unobserve, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("unregisterSession", &smtk::model::Manager::unregisterSession, py::arg("session"), py::arg("expungeSession") = true)
    .def("unusedUUID", &smtk::model::Manager::unusedUUID)
    .def("useOrShellIncludesShells", &smtk::model::Manager::useOrShellIncludesShells, py::arg("cellUseOrShell"))
    .def("pointerAsString", [](smtk::model::Manager &m){
        std::ostringstream result;
        result << &m;
        return result.str();
      })
    ;
  return instance;
}

#endif
