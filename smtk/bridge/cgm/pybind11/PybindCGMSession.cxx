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

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "PybindEngines.h"
#include "PybindExportSolid.h"
#include "PybindImportSolid.h"
#include "PybindOperator.h"
#include "PybindPointerDefs.h"
#include "PybindSession.h"

#include "PybindBooleanIntersection.h"
#include "PybindBooleanSubtraction.h"
#include "PybindBooleanUnion.h"
#include "PybindCopy.h"
#include "PybindCreateBody.h"
#include "PybindCreateBrick.h"
#include "PybindCreateCylinder.h"
#include "PybindCreateEdge.h"
#include "PybindCreateFace.h"
#include "PybindCreatePrism.h"
#include "PybindCreateSphere.h"
#include "PybindCreateVertex.h"
#include "PybindRead.h"
#include "PybindReflect.h"
#include "PybindRemoveModel.h"
#include "PybindRotate.h"
#include "PybindScale.h"
#include "PybindSweep.h"
#include "PybindTranslate.h"
#include "PybindWrite.h"

#include "smtk/model/Operator.h"
#include "smtk/model/Session.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_PLUGIN(_smtkPybindCGMSession)
{
  py::module cgm("_smtkPybindCGMSession", "<description>");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< smtk::bridge::cgm::Engines > smtk_bridge_cgm_Engines = pybind11_init_smtk_bridge_cgm_Engines(cgm);
  py::class_< smtk::bridge::cgm::ExportSolid > smtk_bridge_cgm_ExportSolid = pybind11_init_smtk_bridge_cgm_ExportSolid(cgm);
  py::class_< smtk::bridge::cgm::ImportSolid > smtk_bridge_cgm_ImportSolid = pybind11_init_smtk_bridge_cgm_ImportSolid(cgm);
  PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator > smtk_bridge_cgm_Operator = pybind11_init_smtk_bridge_cgm_Operator(cgm);
  PySharedPtrClass< smtk::bridge::cgm::Session, smtk::model::Session > smtk_bridge_cgm_Session = pybind11_init_smtk_bridge_cgm_Session(cgm);
  PySharedPtrClass< smtk::bridge::cgm::BooleanIntersection > smtk_bridge_cgm_BooleanIntersection = pybind11_init_smtk_bridge_cgm_BooleanIntersection(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::BooleanSubtraction > smtk_bridge_cgm_BooleanSubtraction = pybind11_init_smtk_bridge_cgm_BooleanSubtraction(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::BooleanUnion > smtk_bridge_cgm_BooleanUnion = pybind11_init_smtk_bridge_cgm_BooleanUnion(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::Copy > smtk_bridge_cgm_Copy = pybind11_init_smtk_bridge_cgm_Copy(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::CreateBody > smtk_bridge_cgm_CreateBody = pybind11_init_smtk_bridge_cgm_CreateBody(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::CreateBrick > smtk_bridge_cgm_CreateBrick = pybind11_init_smtk_bridge_cgm_CreateBrick(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::CreateCylinder > smtk_bridge_cgm_CreateCylinder = pybind11_init_smtk_bridge_cgm_CreateCylinder(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::CreateEdge > smtk_bridge_cgm_CreateEdge = pybind11_init_smtk_bridge_cgm_CreateEdge(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::CreateFace > smtk_bridge_cgm_CreateFace = pybind11_init_smtk_bridge_cgm_CreateFace(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::CreatePrism > smtk_bridge_cgm_CreatePrism = pybind11_init_smtk_bridge_cgm_CreatePrism(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::CreateSphere > smtk_bridge_cgm_CreateSphere = pybind11_init_smtk_bridge_cgm_CreateSphere(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::CreateVertex > smtk_bridge_cgm_CreateVertex = pybind11_init_smtk_bridge_cgm_CreateVertex(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::Read > smtk_bridge_cgm_Read = pybind11_init_smtk_bridge_cgm_Read(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::Reflect > smtk_bridge_cgm_Reflect = pybind11_init_smtk_bridge_cgm_Reflect(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::RemoveModel > smtk_bridge_cgm_RemoveModel = pybind11_init_smtk_bridge_cgm_RemoveModel(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::Rotate > smtk_bridge_cgm_Rotate = pybind11_init_smtk_bridge_cgm_Rotate(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::Scale > smtk_bridge_cgm_Scale = pybind11_init_smtk_bridge_cgm_Scale(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::Sweep > smtk_bridge_cgm_Sweep = pybind11_init_smtk_bridge_cgm_Sweep(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::Translate > smtk_bridge_cgm_Translate = pybind11_init_smtk_bridge_cgm_Translate(cgm, smtk_bridge_cgm_Operator);
  PySharedPtrClass< smtk::bridge::cgm::Write > smtk_bridge_cgm_Write = pybind11_init_smtk_bridge_cgm_Write(cgm, smtk_bridge_cgm_Operator);

  return cgm.ptr();
}
