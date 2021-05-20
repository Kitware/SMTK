//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_SessionRef_h
#define pybind_smtk_model_SessionRef_h

#include <pybind11/pybind11.h>

#include "smtk/model/SessionRef.h"

#include "smtk/attribute/Definition.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"
#include "smtk/model/StringData.h"

namespace py = pybind11;

inline py::class_< smtk::model::SessionRef, smtk::model::EntityRef > pybind11_init_smtk_model_SessionRef(py::module &m)
{
  py::class_< smtk::model::SessionRef, smtk::model::EntityRef > instance(m, "SessionRef");
  instance
    .def(py::init<::smtk::model::SessionRef const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ResourcePtr, ::smtk::common::UUID const &>())
    .def(py::init<::smtk::model::ResourcePtr, ::smtk::model::SessionPtr>())
    .def("__ne__", (bool (smtk::model::SessionRef::*)(::smtk::model::EntityRef const &) const) &smtk::model::SessionRef::operator!=)
    .def("deepcopy", (smtk::model::SessionRef & (smtk::model::SessionRef::*)(::smtk::model::SessionRef const &)) &smtk::model::SessionRef::operator=)
    .def("__eq__", (bool (smtk::model::SessionRef::*)(::smtk::model::EntityRef const &) const) &smtk::model::SessionRef::operator==)
    .def("addModel", &smtk::model::SessionRef::addModel, py::arg("mod"))
    .def("close", &smtk::model::SessionRef::close)
    .def("engines", &smtk::model::SessionRef::engines)
    .def("fileTypes", &smtk::model::SessionRef::fileTypes, py::arg("engine") = std::string())
    .def("isValid", (bool (smtk::model::SessionRef::*)() const) &smtk::model::SessionRef::isValid)
    .def("session", &smtk::model::SessionRef::session)
    .def("site", &smtk::model::SessionRef::site)
    .def("tag", &smtk::model::SessionRef::tag)
    ;
  return instance;
}

#endif
