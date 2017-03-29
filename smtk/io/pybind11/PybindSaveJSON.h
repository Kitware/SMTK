//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_SaveJSON_h
#define pybind_smtk_io_SaveJSON_h

#include <pybind11/pybind11.h>

#include "smtk/io/SaveJSON.h"

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

PySharedPtrClass< smtk::io::SaveJSON > pybind11_init_smtk_io_SaveJSON(py::module &m)
{
  PySharedPtrClass< smtk::io::SaveJSON > instance(m, "SaveJSON");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::io::SaveJSON const &>())
    .def("deepcopy", (smtk::io::SaveJSON & (smtk::io::SaveJSON::*)(::smtk::io::SaveJSON const &)) &smtk::io::SaveJSON::operator=)
    .def_static("addMeshesRecord", (int (*)(::smtk::model::ManagerPtr const, ::smtk::common::UUIDs const &, ::cJSON *)) &smtk::io::SaveJSON::addMeshesRecord, py::arg("modelMgr"), py::arg("modelIds"), py::arg("sessionRec"))
    .def_static("addMeshesRecord", (int (*)(::smtk::model::ManagerPtr const, ::smtk::model::Models const &, ::cJSON *)) &smtk::io::SaveJSON::addMeshesRecord, py::arg("modelMgr"), py::arg("inModels"), py::arg("sessionRec"))
    .def_static("addModelsRecord", (int (*)(::smtk::model::ManagerPtr const, ::smtk::common::UUIDs const &, ::cJSON *)) &smtk::io::SaveJSON::addModelsRecord, py::arg("modelMgr"), py::arg("modelIds"), py::arg("sessionRec"))
    .def_static("addModelsRecord", (int (*)(::smtk::model::ManagerPtr const, ::smtk::model::Models const &, ::cJSON *)) &smtk::io::SaveJSON::addModelsRecord, py::arg("modelMgr"), py::arg("models"), py::arg("sessionRec"))
    .def_static("createIntegerArray", &smtk::io::SaveJSON::createIntegerArray, py::arg("arr"))
    .def_static("createRPCRequest", (cJSON * (*)(::std::string const &, ::std::string const &, ::std::string const &)) &smtk::io::SaveJSON::createRPCRequest, py::arg("method"), py::arg("params"), py::arg("reqId"))
    // .def_static("createRPCRequest", (cJSON * (*)(::std::string const &, ::cJSON * &, ::std::string const &, int)) &smtk::io::SaveJSON::createRPCRequest, py::arg("method"), py::arg("params"), py::arg("reqId"), py::arg("paramsType") = 5)
    .def_static("createStringArray", &smtk::io::SaveJSON::createStringArray, py::arg("arr"))
    .def_static("createUUIDArray", &smtk::io::SaveJSON::createUUIDArray, py::arg("arr"))
    .def_static("forDanglingEntities", &smtk::io::SaveJSON::forDanglingEntities, py::arg("sessionId"), py::arg("node"), py::arg("modelMgr"))
    .def_static("forFloatData", &smtk::io::SaveJSON::forFloatData, py::arg("dict"), py::arg("fdata"))
    .def_static("forIntegerData", &smtk::io::SaveJSON::forIntegerData, py::arg("dict"), py::arg("idata"))
    .def_static("forLog", &smtk::io::SaveJSON::forLog, py::arg("logrecordarray"), py::arg("log"), py::arg("start") = 0, py::arg("end") = static_cast<unsigned long>(-1))
    .def_static("forManager", &smtk::io::SaveJSON::forManager, py::arg("body"), py::arg("sess"), py::arg("mesh"), py::arg("modelMgr"), py::arg("sections") = ::smtk::io::JSONFlags::JSON_DEFAULT)
    .def_static("forManagerAnalysis", &smtk::io::SaveJSON::forManagerAnalysis, py::arg("uid"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forManagerArrangement", &smtk::io::SaveJSON::forManagerArrangement, py::arg("entry"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forManagerEntity", &smtk::io::SaveJSON::forManagerEntity, py::arg("entry"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forManagerFloatProperties", &smtk::io::SaveJSON::forManagerFloatProperties, py::arg("uid"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forManagerIntegerProperties", &smtk::io::SaveJSON::forManagerIntegerProperties, py::arg("uid"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forManagerMeshes", &smtk::io::SaveJSON::forManagerMeshes, py::arg("meshes"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forManagerSession", &smtk::io::SaveJSON::forManagerSession, py::arg("sessionId"), py::arg("arg1"), py::arg("modelMgr"), py::arg("writeNativeModels") = false, py::arg("referencePath") = std::string())
    .def_static("forManagerSessionPartial", &smtk::io::SaveJSON::forManagerSessionPartial, py::arg("sessionId"), py::arg("modelIds"), py::arg("arg2"), py::arg("modelMgrId"), py::arg("writeNativeModels") = false, py::arg("referencePath") = std::string())
    .def_static("forManagerStringProperties", &smtk::io::SaveJSON::forManagerStringProperties, py::arg("uid"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forManagerTessellation", &smtk::io::SaveJSON::forManagerTessellation, py::arg("uid"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forMeshCollections", &smtk::io::SaveJSON::forMeshCollections, py::arg("pnode"), py::arg("collectionIds"), py::arg("meshMgr"))
    .def_static("forModelMeshes", &smtk::io::SaveJSON::forModelMeshes, py::arg("modelid"), py::arg("pnode"), py::arg("modelMgr"))
    .def_static("forModelWorker", &smtk::io::SaveJSON::forModelWorker, py::arg("workerDescription"), py::arg("meshTypeIn"), py::arg("meshTypeOut"), py::arg("session"), py::arg("engine"), py::arg("site"), py::arg("root"), py::arg("workerPath"), py::arg("requirementsFileName"))
    .def_static("forOperator", (int (*)(::smtk::model::OperatorSpecification, ::cJSON *)) &smtk::io::SaveJSON::forOperator, py::arg("op"), py::arg("arg1"))
    .def_static("forOperator", (int (*)(::smtk::model::OperatorPtr, ::cJSON *)) &smtk::io::SaveJSON::forOperator, py::arg("op"), py::arg("arg1"))
    .def_static("forOperatorDefinitions", &smtk::io::SaveJSON::forOperatorDefinitions, py::arg("opSys"), py::arg("arg1"))
    .def_static("forOperatorResult", &smtk::io::SaveJSON::forOperatorResult, py::arg("res"), py::arg("arg1"))
    .def_static("forSingleCollection", &smtk::io::SaveJSON::forSingleCollection, py::arg("mdesc"), py::arg("collection"))
    .def_static("forStringData", &smtk::io::SaveJSON::forStringData, py::arg("dict"), py::arg("sdata"))
    .def_static("fromModelManager", (int (*)(::cJSON *, ::smtk::model::ManagerPtr, ::smtk::io::JSONFlags)) &smtk::io::SaveJSON::fromModelManager, py::arg("json"), py::arg("modelMgr"), py::arg("sections") = ::smtk::io::JSONFlags::JSON_DEFAULT)
    .def_static("fromModelManager", (std::string (*)(::smtk::model::ManagerPtr, ::smtk::io::JSONFlags)) &smtk::io::SaveJSON::fromModelManager, py::arg("modelMgr"), py::arg("sections") = ::smtk::io::JSONFlags::JSON_DEFAULT)
    .def_static("fromModelManagerToFile", &smtk::io::SaveJSON::fromModelManagerToFile, py::arg("modelMgr"), py::arg("filename"))
    .def_static("fromUUIDs", &smtk::io::SaveJSON::fromUUIDs, py::arg("uids"))
    ;
  return instance;
}

#endif
