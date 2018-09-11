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
#include "PybindOperation.h"
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

#include "smtk/model/Session.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindCGMSession, cgm)
{
  cgm.doc() = "<description>";

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< smtk::session::cgm::Engines > smtk_session_cgm_Engines = pybind11_init_smtk_session_cgm_Engines(cgm);
  py::class_< smtk::session::cgm::ExportSolid > smtk_session_cgm_ExportSolid = pybind11_init_smtk_session_cgm_ExportSolid(cgm);
  py::class_< smtk::session::cgm::ImportSolid > smtk_session_cgm_ImportSolid = pybind11_init_smtk_session_cgm_ImportSolid(cgm);
  PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation > smtk_session_cgm_Operation = pybind11_init_smtk_session_cgm_Operator(cgm);
  PySharedPtrClass< smtk::session::cgm::Session, smtk::model::Session > smtk_session_cgm_Session = pybind11_init_smtk_session_cgm_Session(cgm);
  PySharedPtrClass< smtk::session::cgm::BooleanIntersection > smtk_session_cgm_BooleanIntersection = pybind11_init_smtk_session_cgm_BooleanIntersection(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::BooleanSubtraction > smtk_session_cgm_BooleanSubtraction = pybind11_init_smtk_session_cgm_BooleanSubtraction(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::BooleanUnion > smtk_session_cgm_BooleanUnion = pybind11_init_smtk_session_cgm_BooleanUnion(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::Copy > smtk_session_cgm_Copy = pybind11_init_smtk_session_cgm_Copy(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::CreateBody > smtk_session_cgm_CreateBody = pybind11_init_smtk_session_cgm_CreateBody(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::CreateBrick > smtk_session_cgm_CreateBrick = pybind11_init_smtk_session_cgm_CreateBrick(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::CreateCylinder > smtk_session_cgm_CreateCylinder = pybind11_init_smtk_session_cgm_CreateCylinder(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::CreateEdge > smtk_session_cgm_CreateEdge = pybind11_init_smtk_session_cgm_CreateEdge(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::CreateFace > smtk_session_cgm_CreateFace = pybind11_init_smtk_session_cgm_CreateFace(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::CreatePrism > smtk_session_cgm_CreatePrism = pybind11_init_smtk_session_cgm_CreatePrism(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::CreateSphere > smtk_session_cgm_CreateSphere = pybind11_init_smtk_session_cgm_CreateSphere(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::CreateVertex > smtk_session_cgm_CreateVertex = pybind11_init_smtk_session_cgm_CreateVertex(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::Read > smtk_session_cgm_Read = pybind11_init_smtk_session_cgm_Read(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::Reflect > smtk_session_cgm_Reflect = pybind11_init_smtk_session_cgm_Reflect(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::RemoveModel > smtk_session_cgm_RemoveModel = pybind11_init_smtk_session_cgm_RemoveModel(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::Rotate > smtk_session_cgm_Rotate = pybind11_init_smtk_session_cgm_Rotate(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::Scale > smtk_session_cgm_Scale = pybind11_init_smtk_session_cgm_Scale(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::Sweep > smtk_session_cgm_Sweep = pybind11_init_smtk_session_cgm_Sweep(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::Translate > smtk_session_cgm_Translate = pybind11_init_smtk_session_cgm_Translate(cgm, smtk_session_cgm_Operation);
  PySharedPtrClass< smtk::session::cgm::Write > smtk_session_cgm_Write = pybind11_init_smtk_session_cgm_Write(cgm, smtk_session_cgm_Operation);
}
