//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Session_h
#define pybind_smtk_model_Session_h

#include <pybind11/pybind11.h>

#include "smtk/model/Session.h"

#include "smtk/attribute/System.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/io/Logger.h"
#include "smtk/mesh/Manager.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"

namespace py = pybind11;

void pybind11_init_smtk_model_SessionInformation(py::module &m)
{
  py::enum_<smtk::model::SessionInformation>(m, "SessionInformation")
    .value("SESSION_ENTITY_TYPE", smtk::model::SessionInformation::SESSION_ENTITY_TYPE)
    .value("SESSION_ENTITY_RELATIONS", smtk::model::SessionInformation::SESSION_ENTITY_RELATIONS)
    .value("SESSION_ARRANGEMENTS", smtk::model::SessionInformation::SESSION_ARRANGEMENTS)
    .value("SESSION_TESSELLATION", smtk::model::SessionInformation::SESSION_TESSELLATION)
    .value("SESSION_FLOAT_PROPERTIES", smtk::model::SessionInformation::SESSION_FLOAT_PROPERTIES)
    .value("SESSION_STRING_PROPERTIES", smtk::model::SessionInformation::SESSION_STRING_PROPERTIES)
    .value("SESSION_INTEGER_PROPERTIES", smtk::model::SessionInformation::SESSION_INTEGER_PROPERTIES)
    .value("SESSION_ATTRIBUTE_ASSOCIATIONS", smtk::model::SessionInformation::SESSION_ATTRIBUTE_ASSOCIATIONS)
    .value("SESSION_USER_DEFINED_PROPERTIES", smtk::model::SessionInformation::SESSION_USER_DEFINED_PROPERTIES)
    .value("SESSION_NOTHING", smtk::model::SessionInformation::SESSION_NOTHING)
    .value("SESSION_ENTITY_RECORD", smtk::model::SessionInformation::SESSION_ENTITY_RECORD)
    .value("SESSION_ENTITY_ARRANGED", smtk::model::SessionInformation::SESSION_ENTITY_ARRANGED)
    .value("SESSION_PROPERTIES", smtk::model::SessionInformation::SESSION_PROPERTIES)
    .value("SESSION_EVERYTHING", smtk::model::SessionInformation::SESSION_EVERYTHING)
    .value("SESSION_EXHAUSTIVE", smtk::model::SessionInformation::SESSION_EXHAUSTIVE)
    .export_values();
}

PySharedPtrClass< smtk::model::Session > pybind11_init_smtk_model_Session(py::module &m)
{
  PySharedPtrClass< smtk::model::Session > instance(m, "Session");
  instance
    .def(py::init<::smtk::model::Session const &>())
    .def("deepcopy", (smtk::model::Session & (smtk::model::Session::*)(::smtk::model::Session const &)) &smtk::model::Session::operator=)
    .def("allSupportedInformation", &smtk::model::Session::allSupportedInformation)
    .def("className", &smtk::model::Session::className)
    .def("classname", &smtk::model::Session::classname)
    .def("danglingEntities", &smtk::model::Session::danglingEntities)
    .def("declareDanglingEntity", &smtk::model::Session::declareDanglingEntity, py::arg("ent"), py::arg("present") = 0)
    .def("findOperatorConstructor", &smtk::model::Session::findOperatorConstructor, py::arg("opName"))
    .def("findOperatorXML", &smtk::model::Session::findOperatorXML, py::arg("opName"))
    .def("inheritsOperators", &smtk::model::Session::inheritsOperators)
    .def("log", &smtk::model::Session::log)
    .def("manager", &smtk::model::Session::manager)
    .def("meshManager", &smtk::model::Session::meshManager)
    .def("name", &smtk::model::Session::name)
    .def("op", &smtk::model::Session::op, py::arg("opName"))
    .def("operatorLabelsMap", &smtk::model::Session::operatorLabelsMap, py::arg("includeAdvanced") = true)
    .def("operatorNames", &smtk::model::Session::operatorNames, py::arg("includeAdvanced") = true)
    .def("operatorConstructors", &smtk::model::Session::operatorConstructors)
    .def("operatorSystem", (smtk::attribute::System * (smtk::model::Session::*)()) &smtk::model::Session::operatorSystem)
    .def("operatorSystem", (smtk::attribute::System const * (smtk::model::Session::*)() const) &smtk::model::Session::operatorSystem)
    .def("registerOperator", &smtk::model::Session::registerOperator, py::arg("opName"), py::arg("opDescrXML"), py::arg("opCtor"))
    .def_static("registerStaticOperator", &smtk::model::Session::registerStaticOperator, py::arg("opName"), py::arg("opDescrXML"), py::arg("opCtor"))
    .def("sessionId", &smtk::model::Session::sessionId)
    .def("setup", &smtk::model::Session::setup, py::arg("optName"), py::arg("optVal"))
    .def_static("staticClassName", &smtk::model::Session::staticClassName)
    .def("transcribe", &smtk::model::Session::transcribe, py::arg("entity"), py::arg("flags"), py::arg("onlyDangling") = true, py::arg("depth") = -1)
    .def("removeGeneratedProperties", &smtk::model::Session::removeGeneratedProperties, py::arg("entity"), py::arg("propFlags"))
    .def("splitProperties", &smtk::model::Session::splitProperties, py::arg("from"), py::arg("to"))
    .def("mergeProperties", &smtk::model::Session::mergeProperties, py::arg("from"), py::arg("to"))
    ;
  return instance;
}

#endif
