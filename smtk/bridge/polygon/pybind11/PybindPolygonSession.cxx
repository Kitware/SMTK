//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <pybind11/pybind11.h>
#include <utility>

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "PybindOperator.h"
#include "PybindSession.h"
#include "PybindSessionIOJSON.h"

#include "smtk/model/Operator.h"
#include "smtk/model/Session.h"
#include "smtk/model/SessionIOJSON.h"

#include "PybindCreateEdge.h"
#include "PybindCreateEdgeFromPoints.h"
#include "PybindCreateEdgeFromVertices.h"
#include "PybindCreateFaces.h"
#include "PybindCreateFacesFromEdges.h"
#include "PybindCreateModel.h"
#include "PybindCreateVertices.h"
#include "PybindDelete.h"
#include "PybindDemoteVertex.h"
#include "PybindExtractContours.h"
#include "PybindForceCreateFace.h"
#include "PybindImport.h"
#include "PybindSplitEdge.h"
#include "PybindTweakEdge.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_PLUGIN(_smtkPybindPolygonSession)
{
  py::module polygon("_smtkPybindPolygonSession", "<description>");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  PySharedPtrClass< smtk::bridge::polygon::Operator, smtk::model::Operator > smtk_bridge_polygon_Operator = pybind11_init_smtk_bridge_polygon_Operator(polygon);
  PySharedPtrClass< smtk::bridge::polygon::Session, smtk::model::Session > smtk_bridge_polygon_Session = pybind11_init_smtk_bridge_polygon_Session(polygon);
  py::class_< smtk::bridge::polygon::SessionIOJSON, smtk::model::SessionIOJSON > smtk_bridge_polygon_SessionIOJSON = pybind11_init_smtk_bridge_polygon_SessionIOJSON(polygon);

  py::class_< smtk::bridge::polygon::ModelEdgeInfo > smtk_bridge_polygon_ModelEdgeInfo = pybind11_init_smtk_bridge_polygon_ModelEdgeInfo(polygon);
  PySharedPtrClass< smtk::bridge::polygon::CreateEdge > smtk_bridge_polygon_CreateEdge = pybind11_init_smtk_bridge_polygon_CreateEdge(polygon, smtk_bridge_polygon_Operator);
  PySharedPtrClass< smtk::bridge::polygon::CreateEdgeFromPoints > smtk_bridge_polygon_CreateEdgeFromPoints = pybind11_init_smtk_bridge_polygon_CreateEdgeFromPoints(polygon, smtk_bridge_polygon_Operator);
  PySharedPtrClass< smtk::bridge::polygon::CreateEdgeFromVertices > smtk_bridge_polygon_CreateEdgeFromVertices = pybind11_init_smtk_bridge_polygon_CreateEdgeFromVertices(polygon, smtk_bridge_polygon_Operator);
  PySharedPtrClass< smtk::bridge::polygon::CreateFaces > smtk_bridge_polygon_CreateFaces = pybind11_init_smtk_bridge_polygon_CreateFaces(polygon, smtk_bridge_polygon_Operator);
  PySharedPtrClass< smtk::bridge::polygon::CreateModel > smtk_bridge_polygon_CreateModel = pybind11_init_smtk_bridge_polygon_CreateModel(polygon, smtk_bridge_polygon_Operator);
  PySharedPtrClass< smtk::bridge::polygon::CreateVertices > smtk_bridge_polygon_CreateVertices = pybind11_init_smtk_bridge_polygon_CreateVertices(polygon, smtk_bridge_polygon_Operator);
  PySharedPtrClass< smtk::bridge::polygon::Delete > smtk_bridge_polygon_Delete = pybind11_init_smtk_bridge_polygon_Delete(polygon, smtk_bridge_polygon_Operator);
  PySharedPtrClass< smtk::bridge::polygon::DemoteVertex > smtk_bridge_polygon_DemoteVertex = pybind11_init_smtk_bridge_polygon_DemoteVertex(polygon, smtk_bridge_polygon_Operator);
  PySharedPtrClass< smtk::bridge::polygon::ExtractContours > smtk_bridge_polygon_ExtractContours = pybind11_init_smtk_bridge_polygon_ExtractContours(polygon, smtk_bridge_polygon_Operator);
  PySharedPtrClass< smtk::bridge::polygon::ForceCreateFace > smtk_bridge_polygon_ForceCreateFace = pybind11_init_smtk_bridge_polygon_ForceCreateFace(polygon, smtk_bridge_polygon_Operator);
  PySharedPtrClass< smtk::bridge::polygon::Import > smtk_bridge_polygon_Import = pybind11_init_smtk_bridge_polygon_Import(polygon, smtk_bridge_polygon_Operator);
  PySharedPtrClass< smtk::bridge::polygon::SplitEdge > smtk_bridge_polygon_SplitEdge = pybind11_init_smtk_bridge_polygon_SplitEdge(polygon, smtk_bridge_polygon_Operator);
  PySharedPtrClass< smtk::bridge::polygon::TweakEdge > smtk_bridge_polygon_TweakEdge = pybind11_init_smtk_bridge_polygon_TweakEdge(polygon, smtk_bridge_polygon_Operator);
  PySharedPtrClass< smtk::bridge::polygon::CreateFacesFromEdges > smtk_bridge_polygon_CreateFacesFromEdges = pybind11_init_smtk_bridge_polygon_CreateFacesFromEdges(polygon, smtk_bridge_polygon_CreateFaces);

  return polygon.ptr();
}
