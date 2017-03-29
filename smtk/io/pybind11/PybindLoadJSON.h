//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_LoadJSON_h
#define pybind_smtk_io_LoadJSON_h

#include <pybind11/pybind11.h>

#include "smtk/io/LoadJSON.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/DefaultSession.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/RemoteOperator.h"
#include "smtk/model/SessionIOJSON.h"
#include "smtk/model/SessionRegistrar.h"
#include "smtk/model/StringData.h"
#include "smtk/model/Tessellation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/System.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/json/Interface.h"
#include "smtk/mesh/json/Readers.h"
#include "smtk/mesh/moab/Interface.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/ImportMesh.h"
#include "smtk/io/Logger.h"

#include "smtk/common/CompilerInformation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::LoadJSON > pybind11_init_smtk_io_LoadJSON(py::module &m)
{
  PySharedPtrClass< smtk::io::LoadJSON > instance(m, "LoadJSON");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::io::LoadJSON const &>())
    .def("deepcopy", (smtk::io::LoadJSON & (smtk::io::LoadJSON::*)(::smtk::io::LoadJSON const &)) &smtk::io::LoadJSON::operator=)
    .def_static("intoModelManager", &smtk::io::LoadJSON::intoModelManager, py::arg("json"), py::arg("manager"))
    .def_static("ofManager", &smtk::io::LoadJSON::ofManager, py::arg("body"), py::arg("manager"))
    .def_static("ofManagerEntity", &smtk::io::LoadJSON::ofManagerEntity, py::arg("uid"), py::arg("arg1"), py::arg("manager"))
    .def_static("ofManagerArrangement", &smtk::io::LoadJSON::ofManagerArrangement, py::arg("uid"), py::arg("arg1"), py::arg("manager"))
    .def_static("ofManagerTessellation", &smtk::io::LoadJSON::ofManagerTessellation, py::arg("uid"), py::arg("arg1"), py::arg("manager"))
    .def_static("ofManagerAnalysis", &smtk::io::LoadJSON::ofManagerAnalysis, py::arg("uid"), py::arg("arg1"), py::arg("manager"))
    .def_static("ofManagerFloatProperties", &smtk::io::LoadJSON::ofManagerFloatProperties, py::arg("uid"), py::arg("arg1"), py::arg("manager"))
    .def_static("ofManagerStringProperties", &smtk::io::LoadJSON::ofManagerStringProperties, py::arg("uid"), py::arg("arg1"), py::arg("manager"))
    .def_static("ofManagerIntegerProperties", &smtk::io::LoadJSON::ofManagerIntegerProperties, py::arg("uid"), py::arg("arg1"), py::arg("manager"))
    .def_static("ofRemoteSession", &smtk::io::LoadJSON::ofRemoteSession, py::arg("arg0"), py::arg("destSession"), py::arg("context"), py::arg("refPath") = std::string())
    .def_static("ofLocalSession", &smtk::io::LoadJSON::ofLocalSession, py::arg("arg0"), py::arg("context"), py::arg("loadNativeModels") = false, py::arg("referencePath") = std::string())
    .def_static("ofOperator", &smtk::io::LoadJSON::ofOperator, py::arg("node"), py::arg("op"), py::arg("context"))
    .def_static("ofOperatorResult", &smtk::io::LoadJSON::ofOperatorResult, py::arg("node"), py::arg("resOut"), py::arg("op"))
    .def_static("ofDanglingEntities", &smtk::io::LoadJSON::ofDanglingEntities, py::arg("node"), py::arg("context"))
    .def_static("ofLog", (int (*)(char const *, ::smtk::io::Logger &)) &smtk::io::LoadJSON::ofLog, py::arg("jsonStr"), py::arg("log"))
    .def_static("ofLog", (int (*)(::cJSON *, ::smtk::io::Logger &)) &smtk::io::LoadJSON::ofLog, py::arg("logrecordarray"), py::arg("log"))
    .def_static("ofMeshesOfModel", &smtk::io::LoadJSON::ofMeshesOfModel, py::arg("node"), py::arg("modelMgr"), py::arg("refPath") = std::string())
    .def_static("ofMeshProperties", &smtk::io::LoadJSON::ofMeshProperties, py::arg("node"), py::arg("collection"))
    .def_static("sessionNameFromTagData", &smtk::io::LoadJSON::sessionNameFromTagData, py::arg("tagData"))
    .def_static("sessionFileTypesFromTagData", &smtk::io::LoadJSON::sessionFileTypesFromTagData, py::arg("tagData"))
    .def_static("getUUIDArrayFromJSON", &smtk::io::LoadJSON::getUUIDArrayFromJSON, py::arg("uidRec"), py::arg("uids"))
    .def_static("getStringArrayFromJSON", &smtk::io::LoadJSON::getStringArrayFromJSON, py::arg("arrayNode"), py::arg("text"))
    .def_static("getIntegerArrayFromJSON", &smtk::io::LoadJSON::getIntegerArrayFromJSON, py::arg("arrayNode"), py::arg("values"))
    .def_static("getRealArrayFromJSON", &smtk::io::LoadJSON::getRealArrayFromJSON, py::arg("arrayNode"), py::arg("values"))
    ;
  return instance;
}

#endif
