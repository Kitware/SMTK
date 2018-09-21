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

#include "PybindOperation.h"
#include "PybindSession.h"
#include "PybindSessionIOJSON.h"

#include "smtk/operation/Operation.h"
#include "smtk/model/Session.h"
#include "smtk/model/SessionIOJSON.h"

#include "PybindCreateEdge.h"
#include "PybindCreateEdgeFromPoints.h"
#include "PybindCreateEdgeFromVertices.h"
#include "PybindCreateFaces.h"
#include "PybindCreateModel.h"
#include "PybindCreateVertices.h"
#include "PybindDelete.h"
#include "PybindDemoteVertex.h"
//#include "PybindExtractContours.h"
#include "PybindForceCreateFace.h"
//#include "PybindImport.h"
#include "PybindRead.h"
#include "PybindLegacyRead.h"
#include "PybindSplitEdge.h"
#include "PybindTweakEdge.h"
#include "PybindWrite.h"

#include "PybindRegistrar.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindPolygonSession, polygon)
{
  polygon.doc() = "<description>";

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation > smtk_session_polygon_Operation = pybind11_init_smtk_session_polygon_Operation(polygon);
  PySharedPtrClass< smtk::session::polygon::Session, smtk::model::Session > smtk_session_polygon_Session = pybind11_init_smtk_session_polygon_Session(polygon);
  py::class_< smtk::session::polygon::SessionIOJSON, smtk::model::SessionIOJSON > smtk_session_polygon_SessionIOJSON = pybind11_init_smtk_session_polygon_SessionIOJSON(polygon);

  py::class_< smtk::session::polygon::ModelEdgeInfo > smtk_session_polygon_ModelEdgeInfo = pybind11_init_smtk_session_polygon_ModelEdgeInfo(polygon);
  PySharedPtrClass< smtk::session::polygon::CreateEdge > smtk_session_polygon_CreateEdge = pybind11_init_smtk_session_polygon_CreateEdge(polygon, smtk_session_polygon_Operation);
  PySharedPtrClass< smtk::session::polygon::CreateEdgeFromPoints > smtk_session_polygon_CreateEdgeFromPoints = pybind11_init_smtk_session_polygon_CreateEdgeFromPoints(polygon, smtk_session_polygon_Operation);
  PySharedPtrClass< smtk::session::polygon::CreateEdgeFromVertices > smtk_session_polygon_CreateEdgeFromVertices = pybind11_init_smtk_session_polygon_CreateEdgeFromVertices(polygon, smtk_session_polygon_Operation);
  PySharedPtrClass< smtk::session::polygon::CreateFaces > smtk_session_polygon_CreateFaces = pybind11_init_smtk_session_polygon_CreateFaces(polygon, smtk_session_polygon_Operation);
  PySharedPtrClass< smtk::session::polygon::CreateModel > smtk_session_polygon_CreateModel = pybind11_init_smtk_session_polygon_CreateModel(polygon, smtk_session_polygon_Operation);
  PySharedPtrClass< smtk::session::polygon::CreateVertices > smtk_session_polygon_CreateVertices = pybind11_init_smtk_session_polygon_CreateVertices(polygon, smtk_session_polygon_Operation);
  PySharedPtrClass< smtk::session::polygon::Delete > smtk_session_polygon_Delete = pybind11_init_smtk_session_polygon_Delete(polygon, smtk_session_polygon_Operation);
  PySharedPtrClass< smtk::session::polygon::DemoteVertex > smtk_session_polygon_DemoteVertex = pybind11_init_smtk_session_polygon_DemoteVertex(polygon, smtk_session_polygon_Operation);
  //PySharedPtrClass< smtk::session::polygon::ExtractContours > smtk_session_polygon_ExtractContours = pybind11_init_smtk_session_polygon_ExtractContours(polygon, smtk_session_polygon_Operation);
  PySharedPtrClass< smtk::session::polygon::ForceCreateFace > smtk_session_polygon_ForceCreateFace = pybind11_init_smtk_session_polygon_ForceCreateFace(polygon, smtk_session_polygon_Operation);
  //PySharedPtrClass< smtk::session::polygon::Import > smtk_session_polygon_Import = pybind11_init_smtk_session_polygon_Import(polygon, smtk_session_polygon_Operation);
  PySharedPtrClass< smtk::session::polygon::Read > smtk_session_polygon_Read = pybind11_init_smtk_session_polygon_Read(polygon, smtk_session_polygon_Operation);
  PySharedPtrClass< smtk::session::polygon::LegacyRead > smtk_session_polygon_LegacyRead = pybind11_init_smtk_session_polygon_LegacyRead(polygon, smtk_session_polygon_Operation);
  PySharedPtrClass< smtk::session::polygon::SplitEdge > smtk_session_polygon_SplitEdge = pybind11_init_smtk_session_polygon_SplitEdge(polygon, smtk_session_polygon_Operation);
  PySharedPtrClass< smtk::session::polygon::TweakEdge > smtk_session_polygon_TweakEdge = pybind11_init_smtk_session_polygon_TweakEdge(polygon, smtk_session_polygon_Operation);
  PySharedPtrClass< smtk::session::polygon::Write > smtk_session_polygon_Write = pybind11_init_smtk_session_polygon_Write(polygon, smtk_session_polygon_Operation);

  py::class_< smtk::session::polygon::Registrar > smtk_session_polygon_Registrar = pybind11_init_smtk_session_polygon_Registrar(polygon);
}
