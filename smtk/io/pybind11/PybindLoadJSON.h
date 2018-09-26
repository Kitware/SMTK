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


#include "smtk/model/Arrangement.h"
#include "smtk/model/DefaultSession.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Resource.h"
#include "smtk/model/SessionIOJSON.h"
#include "smtk/model/SessionRegistrar.h"
#include "smtk/model/StringData.h"
#include "smtk/model/Tessellation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"
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
    .def_static("intoModelResource", &smtk::io::LoadJSON::intoModelResource, py::arg("json"), py::arg("resource"))
    .def_static("ofResource", &smtk::io::LoadJSON::ofResource, py::arg("body"), py::arg("resource"))
    .def_static("ofResourceEntity", &smtk::io::LoadJSON::ofResourceEntity, py::arg("uid"), py::arg("arg1"), py::arg("resource"))
    .def_static("ofResourceArrangement", &smtk::io::LoadJSON::ofResourceArrangement, py::arg("uid"), py::arg("arg1"), py::arg("resource"))
    .def_static("ofResourceTessellation", &smtk::io::LoadJSON::ofResourceTessellation, py::arg("uid"), py::arg("arg1"), py::arg("resource"))
    .def_static("ofResourceAnalysis", &smtk::io::LoadJSON::ofResourceAnalysis, py::arg("uid"), py::arg("arg1"), py::arg("resource"))
    .def_static("ofResourceFloatProperties", &smtk::io::LoadJSON::ofResourceFloatProperties, py::arg("uid"), py::arg("arg1"), py::arg("resource"))
    .def_static("ofResourceStringProperties", &smtk::io::LoadJSON::ofResourceStringProperties, py::arg("uid"), py::arg("arg1"), py::arg("resource"))
    .def_static("ofResourceIntegerProperties", &smtk::io::LoadJSON::ofResourceIntegerProperties, py::arg("uid"), py::arg("arg1"), py::arg("resource"))
    .def_static("ofRemoteSession", &smtk::io::LoadJSON::ofRemoteSession, py::arg("arg0"), py::arg("destSession"), py::arg("context"), py::arg("refPath") = std::string())
    .def_static("ofLocalSession", &smtk::io::LoadJSON::ofLocalSession, py::arg("arg0"), py::arg("context"), py::arg("loadNativeModels") = false, py::arg("referencePath") = std::string())
    .def_static("ofDanglingEntities", &smtk::io::LoadJSON::ofDanglingEntities, py::arg("node"), py::arg("context"))
    .def_static("ofLog", (int (*)(char const *, ::smtk::io::Logger &)) &smtk::io::LoadJSON::ofLog, py::arg("jsonStr"), py::arg("log"))
    .def_static("ofLog", (int (*)(::cJSON *, ::smtk::io::Logger &)) &smtk::io::LoadJSON::ofLog, py::arg("logrecordarray"), py::arg("log"))
    .def_static("ofMeshesOfModel", &smtk::io::LoadJSON::ofMeshesOfModel, py::arg("node"), py::arg("modelResource"), py::arg("refPath") = std::string())
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
