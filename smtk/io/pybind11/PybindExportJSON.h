//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_ExportJSON_h
#define pybind_smtk_io_ExportJSON_h

#include <pybind11/pybind11.h>

#include "smtk/io/ExportJSON.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/model/Operator.h"

namespace py = pybind11;

void pybind11_init_smtk_io_JSONFlags(py::module &m)
{
  py::enum_<smtk::io::JSONFlags>(m, "JSONFlags")
    .value("JSON_NOTHING", smtk::io::JSONFlags::JSON_NOTHING)
    .value("JSON_ENTITIES", smtk::io::JSONFlags::JSON_ENTITIES)
    .value("JSON_SESSIONS", smtk::io::JSONFlags::JSON_SESSIONS)
    .value("JSON_PROPERTIES", smtk::io::JSONFlags::JSON_PROPERTIES)
    .value("JSON_TESSELLATIONS", smtk::io::JSONFlags::JSON_TESSELLATIONS)
    .value("JSON_ANALYSISMESH", smtk::io::JSONFlags::JSON_ANALYSISMESH)
    .value("JSON_MESHES", smtk::io::JSONFlags::JSON_MESHES)
    .value("JSON_CLIENT_DATA", smtk::io::JSONFlags::JSON_CLIENT_DATA)
    .value("JSON_DEFAULT", smtk::io::JSONFlags::JSON_DEFAULT)
    .export_values();
}

PySharedPtrClass< smtk::io::ExportJSON > pybind11_init_smtk_io_ExportJSON(py::module &m)
{
  PySharedPtrClass< smtk::io::ExportJSON > instance(m, "ExportJSON");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::io::ExportJSON const &>())
    .def("deepcopy", (smtk::io::ExportJSON & (smtk::io::ExportJSON::*)(::smtk::io::ExportJSON const &)) &smtk::io::ExportJSON::operator=)
    .def_static("addMeshesRecord", (int (*)(::smtk::model::ManagerPtr const, ::smtk::common::UUIDs const &, ::cJSON *)) &smtk::io::ExportJSON::addMeshesRecord, py::arg("modelMgr"), py::arg("modelIds"), py::arg("sessionRec"))
    .def_static("addMeshesRecord", (int (*)(::smtk::model::ManagerPtr const, ::smtk::model::Models const &, ::cJSON *)) &smtk::io::ExportJSON::addMeshesRecord, py::arg("modelMgr"), py::arg("inModels"), py::arg("sessionRec"))
    .def_static("addModelsRecord", (int (*)(::smtk::model::ManagerPtr const, ::smtk::common::UUIDs const &, ::cJSON *)) &smtk::io::ExportJSON::addModelsRecord, py::arg("modelMgr"), py::arg("modelIds"), py::arg("sessionRec"))
    .def_static("addModelsRecord", (int (*)(::smtk::model::ManagerPtr const, ::smtk::model::Models const &, ::cJSON *)) &smtk::io::ExportJSON::addModelsRecord, py::arg("modelMgr"), py::arg("models"), py::arg("sessionRec"))
    .def_static("createIntegerArray", &smtk::io::ExportJSON::createIntegerArray, py::arg("arr"))
    .def_static("createRPCRequest", (cJSON * (*)(::std::string const &, ::std::string const &, ::std::string const &)) &smtk::io::ExportJSON::createRPCRequest, py::arg("method"), py::arg("params"), py::arg("reqId"))
    // .def_static("createRPCRequest", (cJSON * (*)(::std::string const &, ::cJSON * &, ::std::string const &, int)) &smtk::io::ExportJSON::createRPCRequest, py::arg("method"), py::arg("params"), py::arg("reqId"), py::arg("paramsType") = 5)
    .def_static("createStringArray", &smtk::io::ExportJSON::createStringArray, py::arg("arr"))
    .def_static("createUUIDArray", &smtk::io::ExportJSON::createUUIDArray, py::arg("arr"))
    .def_static("forDanglingEntities", &smtk::io::ExportJSON::forDanglingEntities, py::arg("sessionId"), py::arg("node"), py::arg("modelMgr"))
    .def_static("forFloatData", &smtk::io::ExportJSON::forFloatData, py::arg("dict"), py::arg("fdata"))
    .def_static("forIntegerData", &smtk::io::ExportJSON::forIntegerData, py::arg("dict"), py::arg("idata"))
    .def_static("forLog", &smtk::io::ExportJSON::forLog, py::arg("logrecordarray"), py::arg("log"), py::arg("start") = 0, py::arg("end") = static_cast<unsigned long>(-1))
    .def_static("forManager", &smtk::io::ExportJSON::forManager, py::arg("body"), py::arg("sess"), py::arg("mesh"), py::arg("modelMgr"), py::arg("sections") = ::smtk::io::JSONFlags::JSON_DEFAULT)
    .def_static("forManagerAnalysis", &smtk::io::ExportJSON::forManagerAnalysis, py::arg("uid"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forManagerArrangement", &smtk::io::ExportJSON::forManagerArrangement, py::arg("entry"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forManagerEntity", &smtk::io::ExportJSON::forManagerEntity, py::arg("entry"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forManagerFloatProperties", &smtk::io::ExportJSON::forManagerFloatProperties, py::arg("uid"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forManagerIntegerProperties", &smtk::io::ExportJSON::forManagerIntegerProperties, py::arg("uid"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forManagerMeshes", &smtk::io::ExportJSON::forManagerMeshes, py::arg("meshes"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forManagerSession", &smtk::io::ExportJSON::forManagerSession, py::arg("sessionId"), py::arg("arg1"), py::arg("modelMgr"), py::arg("writeNativeModels") = false, py::arg("referencePath") = std::string())
    .def_static("forManagerSessionPartial", &smtk::io::ExportJSON::forManagerSessionPartial, py::arg("sessionId"), py::arg("modelIds"), py::arg("arg2"), py::arg("modelMgrId"), py::arg("writeNativeModels") = false, py::arg("referencePath") = std::string())
    .def_static("forManagerStringProperties", &smtk::io::ExportJSON::forManagerStringProperties, py::arg("uid"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forManagerTessellation", &smtk::io::ExportJSON::forManagerTessellation, py::arg("uid"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forMeshCollections", &smtk::io::ExportJSON::forMeshCollections, py::arg("pnode"), py::arg("collectionIds"), py::arg("meshMgr"))
    .def_static("forModelMeshes", &smtk::io::ExportJSON::forModelMeshes, py::arg("modelid"), py::arg("pnode"), py::arg("modelMgr"))
    .def_static("forModelWorker", &smtk::io::ExportJSON::forModelWorker, py::arg("workerDescription"), py::arg("meshTypeIn"), py::arg("meshTypeOut"), py::arg("session"), py::arg("engine"), py::arg("site"), py::arg("root"), py::arg("workerPath"), py::arg("requirementsFileName"))
    .def_static("forOperator", (int (*)(::smtk::model::OperatorSpecification, ::cJSON *)) &smtk::io::ExportJSON::forOperator, py::arg("op"), py::arg("arg1"))
    .def_static("forOperator", (int (*)(::smtk::model::OperatorPtr, ::cJSON *)) &smtk::io::ExportJSON::forOperator, py::arg("op"), py::arg("arg1"))
    .def_static("forOperatorDefinitions", &smtk::io::ExportJSON::forOperatorDefinitions, py::arg("opSys"), py::arg("arg1"))
    .def_static("forOperatorResult", &smtk::io::ExportJSON::forOperatorResult, py::arg("res"), py::arg("arg1"))
    .def_static("forSingleCollection", &smtk::io::ExportJSON::forSingleCollection, py::arg("mdesc"), py::arg("collection"))
    .def_static("forStringData", &smtk::io::ExportJSON::forStringData, py::arg("dict"), py::arg("sdata"))
    .def_static("fromModelManager", (int (*)(::cJSON *, ::smtk::model::ManagerPtr, ::smtk::io::JSONFlags)) &smtk::io::ExportJSON::fromModelManager, py::arg("json"), py::arg("modelMgr"), py::arg("sections") = ::smtk::io::JSONFlags::JSON_DEFAULT)
    .def_static("fromModelManager", (std::string (*)(::smtk::model::ManagerPtr, ::smtk::io::JSONFlags)) &smtk::io::ExportJSON::fromModelManager, py::arg("modelMgr"), py::arg("sections") = ::smtk::io::JSONFlags::JSON_DEFAULT)
    .def_static("fromModelManagerToFile", &smtk::io::ExportJSON::fromModelManagerToFile, py::arg("modelMgr"), py::arg("filename"))
    .def_static("fromUUIDs", &smtk::io::ExportJSON::fromUUIDs, py::arg("uids"))
    ;
  return instance;
}

#endif
