//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_ImportJSON_h
#define pybind_smtk_io_ImportJSON_h

#include <pybind11/pybind11.h>

#include "smtk/io/ImportJSON.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/SessionRegistrar.h"
#include "smtk/model/SessionIOJSON.h"
#include "smtk/model/DefaultSession.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/RemoteOperator.h"
#include "smtk/model/StringData.h"
#include "smtk/model/Tessellation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/System.h"

#include "smtk/mesh/Manager.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/json/Interface.h"
#include "smtk/mesh/json/Readers.h"
#include "smtk/mesh/moab/Interface.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/io/ImportMesh.h"

#include "smtk/common/CompilerInformation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::ImportJSON > pybind11_init_smtk_io_ImportJSON(py::module &m)
{
  PySharedPtrClass< smtk::io::ImportJSON > instance(m, "ImportJSON");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::io::ImportJSON const &>())
    .def("deepcopy", (smtk::io::ImportJSON & (smtk::io::ImportJSON::*)(::smtk::io::ImportJSON const &)) &smtk::io::ImportJSON::operator=)
    .def_static("intoModelManager", &smtk::io::ImportJSON::intoModelManager, py::arg("json"), py::arg("manager"))
    .def_static("ofManager", &smtk::io::ImportJSON::ofManager, py::arg("body"), py::arg("manager"))
    .def_static("ofManagerEntity", &smtk::io::ImportJSON::ofManagerEntity, py::arg("uid"), py::arg("arg1"), py::arg("manager"))
    .def_static("ofManagerArrangement", &smtk::io::ImportJSON::ofManagerArrangement, py::arg("uid"), py::arg("arg1"), py::arg("manager"))
    .def_static("ofManagerTessellation", &smtk::io::ImportJSON::ofManagerTessellation, py::arg("uid"), py::arg("arg1"), py::arg("manager"))
    .def_static("ofManagerAnalysis", &smtk::io::ImportJSON::ofManagerAnalysis, py::arg("uid"), py::arg("arg1"), py::arg("manager"))
    .def_static("ofManagerFloatProperties", &smtk::io::ImportJSON::ofManagerFloatProperties, py::arg("uid"), py::arg("arg1"), py::arg("manager"))
    .def_static("ofManagerStringProperties", &smtk::io::ImportJSON::ofManagerStringProperties, py::arg("uid"), py::arg("arg1"), py::arg("manager"))
    .def_static("ofManagerIntegerProperties", &smtk::io::ImportJSON::ofManagerIntegerProperties, py::arg("uid"), py::arg("arg1"), py::arg("manager"))
    .def_static("ofRemoteSession", &smtk::io::ImportJSON::ofRemoteSession, py::arg("arg0"), py::arg("destSession"), py::arg("context"), py::arg("refPath") = std::string())
    .def_static("ofLocalSession", &smtk::io::ImportJSON::ofLocalSession, py::arg("arg0"), py::arg("context"), py::arg("loadNativeModels") = false, py::arg("referencePath") = std::string())
    .def_static("ofOperator", &smtk::io::ImportJSON::ofOperator, py::arg("node"), py::arg("op"), py::arg("context"))
    .def_static("ofOperatorResult", &smtk::io::ImportJSON::ofOperatorResult, py::arg("node"), py::arg("resOut"), py::arg("op"))
    .def_static("ofDanglingEntities", &smtk::io::ImportJSON::ofDanglingEntities, py::arg("node"), py::arg("context"))
    .def_static("ofLog", (int (*)(char const *, ::smtk::io::Logger &)) &smtk::io::ImportJSON::ofLog, py::arg("jsonStr"), py::arg("log"))
    .def_static("ofLog", (int (*)(::cJSON *, ::smtk::io::Logger &)) &smtk::io::ImportJSON::ofLog, py::arg("logrecordarray"), py::arg("log"))
    .def_static("ofMeshesOfModel", &smtk::io::ImportJSON::ofMeshesOfModel, py::arg("node"), py::arg("modelMgr"), py::arg("refPath") = std::string())
    .def_static("ofMeshProperties", &smtk::io::ImportJSON::ofMeshProperties, py::arg("node"), py::arg("collection"))
    .def_static("sessionNameFromTagData", &smtk::io::ImportJSON::sessionNameFromTagData, py::arg("tagData"))
    .def_static("sessionFileTypesFromTagData", &smtk::io::ImportJSON::sessionFileTypesFromTagData, py::arg("tagData"))
    .def_static("getUUIDArrayFromJSON", &smtk::io::ImportJSON::getUUIDArrayFromJSON, py::arg("uidRec"), py::arg("uids"))
    .def_static("getStringArrayFromJSON", &smtk::io::ImportJSON::getStringArrayFromJSON, py::arg("arrayNode"), py::arg("text"))
    .def_static("getIntegerArrayFromJSON", &smtk::io::ImportJSON::getIntegerArrayFromJSON, py::arg("arrayNode"), py::arg("values"))
    .def_static("getRealArrayFromJSON", &smtk::io::ImportJSON::getRealArrayFromJSON, py::arg("arrayNode"), py::arg("values"))
    ;
  return instance;
}

#endif
