//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_GridInfo_h
#define pybind_smtk_model_GridInfo_h

#include <pybind11/pybind11.h>

#include "smtk/model/GridInfo.h"

namespace py = pybind11;

py::class_< smtk::model::GridInfo > pybind11_init_smtk_model_GridInfo(py::module &m)
{
  py::class_< smtk::model::GridInfo > instance(m, "GridInfo");
  instance
    .def("deepcopy", (smtk::model::GridInfo & (smtk::model::GridInfo::*)(::smtk::model::GridInfo const &)) &smtk::model::GridInfo::operator=)
    .def("dimension", &smtk::model::GridInfo::dimension, py::arg("status"))
    .def("analysisGridCells", &smtk::model::GridInfo::analysisGridCells, py::arg("modelEntityId"), py::arg("status"))
    .def("boundaryItemsOf", &smtk::model::GridInfo::boundaryItemsOf, py::arg("modelEntityId"), py::arg("status"))
    .def("asBoundaryItems", &smtk::model::GridInfo::asBoundaryItems, py::arg("modelEntityId"), py::arg("boundedModelId"), py::arg("status"))
    .def("cellType", &smtk::model::GridInfo::cellType, py::arg("gridCellId"), py::arg("status"))
    .def("pointIds", &smtk::model::GridInfo::pointIds, py::arg("modelEntityId"), py::arg("closure"), py::arg("status"))
    .def("cellPointIds", &smtk::model::GridInfo::cellPointIds, py::arg("gridCellId"), py::arg("status"))
    .def("pointLocation", &smtk::model::GridInfo::pointLocation, py::arg("gridPointId"), py::arg("status"))
    .def("nodeElemSetClassification", &smtk::model::GridInfo::nodeElemSetClassification, py::arg("modelEntityId"), py::arg("status"))
    .def("sideSetClassification", &smtk::model::GridInfo::sideSetClassification, py::arg("modelEntityId"), py::arg("status"))
    .def("edgeGridItems", &smtk::model::GridInfo::edgeGridItems, py::arg("boundaryGroupId"), py::arg("status"))
    ;
  py::enum_<smtk::model::GridInfo::ApiReturnType>(instance, "ApiReturnType")
    .value("OK", smtk::model::GridInfo::ApiReturnType::OK)
    .value("ENTITY_NOT_FOUND", smtk::model::GridInfo::ApiReturnType::ENTITY_NOT_FOUND)
    .value("IDENTIFIER_NOT_FOUND", smtk::model::GridInfo::ApiReturnType::IDENTIFIER_NOT_FOUND)
    .value("NOT_AVAILABLE", smtk::model::GridInfo::ApiReturnType::NOT_AVAILABLE)
    .export_values();
  py::enum_<smtk::model::GridInfo::PointClosure>(instance, "PointClosure")
    .value("ALL_POINTS", smtk::model::GridInfo::PointClosure::ALL_POINTS)
    .value("INTERIOR_POINTS", smtk::model::GridInfo::PointClosure::INTERIOR_POINTS)
    .value("BOUNDARY_POINTS", smtk::model::GridInfo::PointClosure::BOUNDARY_POINTS)
    .export_values();
  py::class_< smtk::model::GridInfo::ApiStatus >(instance, "ApiStatus")
    .def(py::init<>())
    .def(py::init<::smtk::model::GridInfo::ApiStatus const &>())
    .def("deepcopy", (smtk::model::GridInfo::ApiStatus & (smtk::model::GridInfo::ApiStatus::*)(::smtk::model::GridInfo::ApiStatus const &)) &smtk::model::GridInfo::ApiStatus::operator=)
    .def_readwrite("returnType", &smtk::model::GridInfo::ApiStatus::returnType)
    .def_readwrite("errorMessage", &smtk::model::GridInfo::ApiStatus::errorMessage)
    .def_readwrite("logger", &smtk::model::GridInfo::ApiStatus::logger)
    ;
  return instance;
}

#endif
