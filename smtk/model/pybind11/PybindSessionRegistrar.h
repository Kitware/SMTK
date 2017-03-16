//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_SessionRegistrar_h
#define pybind_smtk_model_SessionRegistrar_h

#include <pybind11/pybind11.h>

#include "smtk/model/SessionRegistrar.h"

#include "smtk/model/Session.h"
#include "smtk/model/StringData.h"

namespace py = pybind11;

py::class_< smtk::model::SessionRegistrar > pybind11_init_smtk_model_SessionRegistrar(py::module &m)
{
  py::class_< smtk::model::SessionRegistrar > instance(m, "SessionRegistrar");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::SessionRegistrar const &>())
    .def("deepcopy", (smtk::model::SessionRegistrar & (smtk::model::SessionRegistrar::*)(::smtk::model::SessionRegistrar const &)) &smtk::model::SessionRegistrar::operator=)
    .def_static("createSession", &smtk::model::SessionRegistrar::createSession, py::arg("bname"))
    .def_static("fileTypesTag", &smtk::model::SessionRegistrar::fileTypesTag)
    .def_static("registerSession", &smtk::model::SessionRegistrar::registerSession, py::arg("sname"), py::arg("stags"), py::arg("ssetup"), py::arg("sctor"), py::arg("sopcons"))
    .def_static("sessionConstructor", &smtk::model::SessionRegistrar::sessionConstructor, py::arg("bname"))
    .def_static("sessionEngines", &smtk::model::SessionRegistrar::sessionEngines, py::arg("bname"))
    .def_static("sessionFileTypes", &smtk::model::SessionRegistrar::sessionFileTypes, py::arg("bname"), py::arg("engine") = std::string())
    .def_static("sessionSite", &smtk::model::SessionRegistrar::sessionSite, py::arg("bname"))
    .def_static("sessionStaticSetup", &smtk::model::SessionRegistrar::sessionStaticSetup, py::arg("bname"))
    .def_static("sessionTags", &smtk::model::SessionRegistrar::sessionTags, py::arg("bname"))
    .def_static("sessionTypeNames", &smtk::model::SessionRegistrar::sessionTypeNames)
    ;
  return instance;
}

py::class_< smtk::model::StaticSessionInfo > pybind11_init_smtk_model_StaticSessionInfo(py::module &m)
{
  py::class_< smtk::model::StaticSessionInfo > instance(m, "StaticSessionInfo");
  instance
    .def(py::init<::smtk::model::StaticSessionInfo const &>())
    .def(py::init<>())
    .def(py::init<::std::string const &, ::std::string const &, ::smtk::model::SessionStaticSetup, ::smtk::model::SessionConstructor, ::smtk::model::OperatorConstructors*>())
    .def("deepcopy", (smtk::model::StaticSessionInfo & (smtk::model::StaticSessionInfo::*)(::smtk::model::StaticSessionInfo const &)) &smtk::model::StaticSessionInfo::operator=)
    .def_readwrite("Constructor", &smtk::model::StaticSessionInfo::Constructor)
    .def_readwrite("OpConstructors", &smtk::model::StaticSessionInfo::OpConstructors)
    .def_readwrite("Engines", &smtk::model::StaticSessionInfo::Engines)
    .def_readwrite("FileTypes", &smtk::model::StaticSessionInfo::FileTypes)
    .def_readwrite("Name", &smtk::model::StaticSessionInfo::Name)
    .def_readwrite("Setup", &smtk::model::StaticSessionInfo::Setup)
    .def_readwrite("Site", &smtk::model::StaticSessionInfo::Site)
    .def_readwrite("Tags", &smtk::model::StaticSessionInfo::Tags)
    .def_readwrite("TagsParsed", &smtk::model::StaticSessionInfo::TagsParsed)
    ;
  return instance;
}

void pybind11_init_smtk_model_SessionHasNoStaticSetup(py::module &m)
{
  m.def("SessionHasNoStaticSetup", &smtk::model::SessionHasNoStaticSetup, "", py::arg("arg0"), py::arg("arg1"));
}

#endif
