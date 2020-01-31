//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/pybind11.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <utility>

namespace py = pybind11;

template<typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "smtk/attribute/Resource.h"

#include "PybindArrangement.h"
#include "PybindArrangementHelper.h"
#include "PybindArrangementKind.h"
#include "PybindAttributeAssignments.h"
#include "PybindAuxiliaryGeometry.h"
#include "PybindCellEntity.h"
#include "PybindChain.h"
#include "PybindDefaultSession.h"
#include "PybindDelete.h"
#include "PybindEdge.h"
#include "PybindEdgeUse.h"
#include "PybindEntity.h"
#include "PybindEntityIterator.h"
#include "PybindEntityRef.h"
#include "PybindEntityRefArrangementOps.h"
#include "PybindEntityTypeBits.h"
#include "PybindEvents.h"
#include "PybindFace.h"
#include "PybindFaceUse.h"
#include "PybindFloatData.h"
#include "PybindGroup.h"
#include "PybindInstance.h"
#include "PybindIntegerData.h"
#include "PybindLoop.h"
#include "PybindModel.h"
#include "PybindRegistrar.h"
#include "PybindResource.h"
#include "PybindSession.h"
#include "PybindSessionIO.h"
#include "PybindSessionIOJSON.h"
#include "PybindSessionRef.h"
#include "PybindShell.h"
#include "PybindShellEntity.h"
#include "PybindStringData.h"
#include "PybindTessellation.h"
#include "PybindUseEntity.h"
#include "PybindVertex.h"
#include "PybindVertexUse.h"
#include "PybindVolume.h"
#include "PybindVolumeUse.h"

#include "PybindAddAuxiliaryGeometry.h"
#include "PybindCloseModel.h"
#include "PybindCreateInstances.h"
#include "PybindExportModelJSON.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindModel, model)
{
  model.doc() = "<description>";

  py::module::import("smtk.resource");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_<smtk::model::Arrangement> smtk_model_Arrangement =
    pybind11_init_smtk_model_Arrangement(model);
  py::class_<smtk::model::ArrangementHelper> smtk_model_ArrangementHelper =
    pybind11_init_smtk_model_ArrangementHelper(model);
  py::class_<smtk::model::ArrangementReference> smtk_model_ArrangementReference =
    pybind11_init_smtk_model_ArrangementReference(model);
  py::class_<smtk::model::AttributeAssignments> smtk_model_AttributeAssignments =
    pybind11_init_smtk_model_AttributeAssignments(model);
  py::class_<smtk::model::Entity> smtk_model_Entity = pybind11_init_smtk_model_Entity(model);
  py::class_<smtk::model::EntityIterator> smtk_model_EntityIterator =
    pybind11_init_smtk_model_EntityIterator(model);
  pybind11_init_smtk_model_Exclusions(model);
  py::class_<smtk::model::EntityRef> smtk_model_EntityRef =
    pybind11_init_smtk_model_EntityRef(model);
  py::class_<smtk::model::EntityRefArrangementOps> smtk_model_EntityRefArrangementOps =
    pybind11_init_smtk_model_EntityRefArrangementOps(model);
  PySharedPtrClass<smtk::model::Resource> smtk_model_Resource =
    pybind11_init_smtk_model_Resource(model);
  PySharedPtrClass<smtk::model::Session> smtk_model_Session =
    pybind11_init_smtk_model_Session(model);
  py::class_<smtk::model::SessionIO> smtk_model_SessionIO =
    pybind11_init_smtk_model_SessionIO(model);
  py::class_<smtk::model::Tessellation> smtk_model_Tessellation =
    pybind11_init_smtk_model_Tessellation(model);
  pybind11_init_smtk_model_ArrangementKind(model);
  pybind11_init_smtk_model_EntityTypeBits(model);
  pybind11_init_smtk_model_IteratorStyle(model);
  pybind11_init_smtk_model_ResourceEventChangeType(model);
  pybind11_init_smtk_model_ResourceEventRelationType(model);
  pybind11_init_smtk_model_ModelGeometryStyle(model);
  pybind11_init_smtk_model_Orientation(model);
  pybind11_init_smtk_model_SessionInformation(model);
  pybind11_init_smtk_model_TessellationCellType(model);
  pybind11_init_smtk_model_AbbreviationForArrangementKind(model);
  pybind11_init_smtk_model_ArrangementKindFromAbbreviation(model);
  pybind11_init_smtk_model_ArrangementKindFromName(model);
  pybind11_init_smtk_model_Dual(model);
  pybind11_init_smtk_model_ModelGeometryStyleName(model);
  pybind11_init_smtk_model_NameForArrangementKind(model);
  pybind11_init_smtk_model_NamedModelGeometryStyle(model);
  pybind11_init_smtk_model_entityrefHash(model);
  pybind11_init_smtk_model_isAuxiliaryGeometry(model);
  pybind11_init_smtk_model_isCellEntity(model);
  pybind11_init_smtk_model_isChain(model);
  pybind11_init_smtk_model_isConcept(model);
  pybind11_init_smtk_model_isEdge(model);
  pybind11_init_smtk_model_isEdgeUse(model);
  pybind11_init_smtk_model_isFace(model);
  pybind11_init_smtk_model_isFaceUse(model);
  pybind11_init_smtk_model_isGroup(model);
  pybind11_init_smtk_model_isInstance(model);
  pybind11_init_smtk_model_isLoop(model);
  pybind11_init_smtk_model_isModel(model);
  pybind11_init_smtk_model_isSessionRef(model);
  pybind11_init_smtk_model_isShell(model);
  pybind11_init_smtk_model_isShellEntity(model);
  pybind11_init_smtk_model_isUseEntity(model);
  pybind11_init_smtk_model_isVertex(model);
  pybind11_init_smtk_model_isVertexUse(model);
  pybind11_init_smtk_model_isVolume(model);
  pybind11_init_smtk_model_isVolumeUse(model);
  py::class_<smtk::model::AuxiliaryGeometry, smtk::model::EntityRef> smtk_model_AuxiliaryGeometry =
    pybind11_init_smtk_model_AuxiliaryGeometry(model);
  py::class_<smtk::model::CellEntity, smtk::model::EntityRef> smtk_model_CellEntity =
    pybind11_init_smtk_model_CellEntity(model);
  PySharedPtrClass<smtk::model::DefaultSession, smtk::model::Session> smtk_model_DefaultSession =
    pybind11_init_smtk_model_DefaultSession(model);
  py::class_<smtk::model::Group, smtk::model::EntityRef> smtk_model_Group =
    pybind11_init_smtk_model_Group(model);
  py::class_<smtk::model::Instance, smtk::model::EntityRef> smtk_model_Instance =
    pybind11_init_smtk_model_Instance(model);
  py::class_<smtk::model::Model, smtk::model::EntityRef> smtk_model_Model =
    pybind11_init_smtk_model_Model(model);
  py::class_<smtk::model::SessionIOJSON, smtk::model::SessionIO> smtk_model_SessionIOJSON =
    pybind11_init_smtk_model_SessionIOJSON(model);
  py::class_<smtk::model::SessionRef, smtk::model::EntityRef> smtk_model_SessionRef =
    pybind11_init_smtk_model_SessionRef(model);
  py::class_<smtk::model::ShellEntity, smtk::model::EntityRef> smtk_model_ShellEntity =
    pybind11_init_smtk_model_ShellEntity(model);
  py::class_<smtk::model::UseEntity, smtk::model::EntityRef> smtk_model_UseEntity =
    pybind11_init_smtk_model_UseEntity(model);
  py::class_<smtk::model::Chain, smtk::model::ShellEntity> smtk_model_Chain =
    pybind11_init_smtk_model_Chain(model);
  py::class_<smtk::model::Edge, smtk::model::CellEntity> smtk_model_Edge =
    pybind11_init_smtk_model_Edge(model);
  py::class_<smtk::model::EdgeUse, smtk::model::UseEntity> smtk_model_EdgeUse =
    pybind11_init_smtk_model_EdgeUse(model);
  py::class_<smtk::model::Face, smtk::model::CellEntity> smtk_model_Face =
    pybind11_init_smtk_model_Face(model);
  py::class_<smtk::model::FaceUse, smtk::model::UseEntity> smtk_model_FaceUse =
    pybind11_init_smtk_model_FaceUse(model);
  py::class_<smtk::model::Loop, smtk::model::ShellEntity> smtk_model_Loop =
    pybind11_init_smtk_model_Loop(model);
  py::class_<smtk::model::Shell, smtk::model::ShellEntity> smtk_model_Shell =
    pybind11_init_smtk_model_Shell(model);
  py::class_<smtk::model::Vertex, smtk::model::CellEntity> smtk_model_Vertex =
    pybind11_init_smtk_model_Vertex(model);
  py::class_<smtk::model::VertexUse, smtk::model::UseEntity> smtk_model_VertexUse =
    pybind11_init_smtk_model_VertexUse(model);
  py::class_<smtk::model::Volume, smtk::model::CellEntity> smtk_model_Volume =
    pybind11_init_smtk_model_Volume(model);
  py::class_<smtk::model::VolumeUse, smtk::model::UseEntity> smtk_model_VolumeUse =
    pybind11_init_smtk_model_VolumeUse(model);

  PySharedPtrClass<smtk::model::AddAuxiliaryGeometry, smtk::operation::XMLOperation>
    smtk_model_AddAuxiliaryGeometry = pybind11_init_smtk_model_AddAuxiliaryGeometry(model);
  PySharedPtrClass<smtk::model::CloseModel, smtk::operation::XMLOperation> smtk_model_CloseModel =
    pybind11_init_smtk_model_CloseModel(model);
  PySharedPtrClass<smtk::model::CreateInstances, smtk::operation::XMLOperation>
    smtk_model_CreateInstances = pybind11_init_smtk_model_CreateInstances(model);
  PySharedPtrClass<smtk::model::Delete, smtk::operation::XMLOperation> smtk_model_Delete =
    pybind11_init_smtk_model_Delete(model);
  PySharedPtrClass<smtk::model::ExportModelJSON, smtk::operation::XMLOperation>
    smtk_model_ExportModelJSON = pybind11_init_smtk_model_ExportModelJSON(model);

  py::class_<smtk::model::Registrar> smtk_model_Registrar =
    pybind11_init_smtk_model_Registrar(model);
}
