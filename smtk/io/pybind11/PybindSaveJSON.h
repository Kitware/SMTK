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

#include "smtk/mesh/core/Resource.h"


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
    .def_static("addMeshesRecord", (int (*)(::smtk::model::ResourcePtr const, ::smtk::common::UUIDs const &, ::cJSON *)) &smtk::io::SaveJSON::addMeshesRecord, py::arg("modelMgr"), py::arg("modelIds"), py::arg("sessionRec"))
    .def_static("addMeshesRecord", (int (*)(::smtk::model::ResourcePtr const, ::smtk::model::Models const &, ::cJSON *)) &smtk::io::SaveJSON::addMeshesRecord, py::arg("modelMgr"), py::arg("inModels"), py::arg("sessionRec"))
    .def_static("addModelsRecord", (int (*)(::smtk::model::ResourcePtr const, ::smtk::common::UUIDs const &, ::cJSON *)) &smtk::io::SaveJSON::addModelsRecord, py::arg("modelMgr"), py::arg("modelIds"), py::arg("sessionRec"))
    .def_static("addModelsRecord", (int (*)(::smtk::model::ResourcePtr const, ::smtk::model::Models const &, ::cJSON *)) &smtk::io::SaveJSON::addModelsRecord, py::arg("modelMgr"), py::arg("models"), py::arg("sessionRec"))
    .def_static("createIntegerArray", &smtk::io::SaveJSON::createIntegerArray, py::arg("arr"))
    .def_static("createRPCRequest", (cJSON * (*)(::std::string const &, ::std::string const &, ::std::string const &)) &smtk::io::SaveJSON::createRPCRequest, py::arg("method"), py::arg("params"), py::arg("reqId"))
    // .def_static("createRPCRequest", (cJSON * (*)(::std::string const &, ::cJSON * &, ::std::string const &, int)) &smtk::io::SaveJSON::createRPCRequest, py::arg("method"), py::arg("params"), py::arg("reqId"), py::arg("paramsType") = 5)
    .def_static("createStringArray", &smtk::io::SaveJSON::createStringArray, py::arg("arr"))
    .def_static("createUUIDArray", &smtk::io::SaveJSON::createUUIDArray, py::arg("arr"))
    .def_static("forDanglingEntities", &smtk::io::SaveJSON::forDanglingEntities, py::arg("sessionId"), py::arg("node"), py::arg("modelMgr"))
    .def_static("forFloatData", &smtk::io::SaveJSON::forFloatData, py::arg("dict"), py::arg("fdata"))
    .def_static("forIntegerData", &smtk::io::SaveJSON::forIntegerData, py::arg("dict"), py::arg("idata"))
    .def_static("forLog", &smtk::io::SaveJSON::forLog, py::arg("logrecordarray"), py::arg("log"), py::arg("start") = 0, py::arg("end") = static_cast<unsigned long>(-1))
    .def_static("forResource", &smtk::io::SaveJSON::forResource, py::arg("body"), py::arg("sess"), py::arg("mesh"), py::arg("modelMgr"), py::arg("sections") = ::smtk::io::JSONFlags::JSON_DEFAULT)
    .def_static("forResourceAnalysis", &smtk::io::SaveJSON::forResourceAnalysis, py::arg("uid"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forResourceEntity", &smtk::io::SaveJSON::forResourceEntity, py::arg("entry"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forResourceFloatProperties", &smtk::io::SaveJSON::forResourceFloatProperties, py::arg("uid"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forResourceIntegerProperties", &smtk::io::SaveJSON::forResourceIntegerProperties, py::arg("uid"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forResourceMeshes", &smtk::io::SaveJSON::forResourceMeshes, py::arg("meshes"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forResourceSession", &smtk::io::SaveJSON::forResourceSession, py::arg("sessionId"), py::arg("arg1"), py::arg("modelMgr"), py::arg("writeNativeModels") = false, py::arg("referencePath") = std::string())
    .def_static("forResourceSessionPartial", &smtk::io::SaveJSON::forResourceSessionPartial, py::arg("sessionId"), py::arg("modelIds"), py::arg("arg2"), py::arg("modelMgrId"), py::arg("writeNativeModels") = false, py::arg("referencePath") = std::string())
    .def_static("forResourceStringProperties", &smtk::io::SaveJSON::forResourceStringProperties, py::arg("uid"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forResourceTessellation", &smtk::io::SaveJSON::forResourceTessellation, py::arg("uid"), py::arg("arg1"), py::arg("modelMgr"))
    .def_static("forMeshResources", &smtk::io::SaveJSON::forMeshResources, py::arg("pnode"), py::arg("resourceIds"), py::arg("meshMgr"))
    .def_static("forModelMeshes", &smtk::io::SaveJSON::forModelMeshes, py::arg("modelid"), py::arg("pnode"), py::arg("modelMgr"))
    .def_static("forModelWorker", &smtk::io::SaveJSON::forModelWorker, py::arg("workerDescription"), py::arg("meshTypeIn"), py::arg("meshTypeOut"), py::arg("session"), py::arg("engine"), py::arg("site"), py::arg("root"), py::arg("workerPath"), py::arg("requirementsFileName"))
    .def_static("forSingleResource", &smtk::io::SaveJSON::forSingleResource, py::arg("mdesc"), py::arg("resource"))
    .def_static("forStringData", &smtk::io::SaveJSON::forStringData, py::arg("dict"), py::arg("sdata"))
    .def_static("fromModelResource", (int (*)(::cJSON *, ::smtk::model::ResourcePtr, ::smtk::io::JSONFlags)) &smtk::io::SaveJSON::fromModelResource, py::arg("json"), py::arg("modelMgr"), py::arg("sections") = ::smtk::io::JSONFlags::JSON_DEFAULT)
    .def_static("fromModelResource", (std::string (*)(::smtk::model::ResourcePtr, ::smtk::io::JSONFlags)) &smtk::io::SaveJSON::fromModelResource, py::arg("modelMgr"), py::arg("sections") = ::smtk::io::JSONFlags::JSON_DEFAULT)
    .def_static("fromModelResourceToFile", &smtk::io::SaveJSON::fromModelResourceToFile, py::arg("modelMgr"), py::arg("filename"))
    .def_static("fromUUIDs", &smtk::io::SaveJSON::fromUUIDs, py::arg("uids"))
    ;
  return instance;
}

#endif
