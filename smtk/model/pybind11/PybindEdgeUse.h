//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_EdgeUse_h
#define pybind_smtk_model_EdgeUse_h

#include <pybind11/pybind11.h>

#include "smtk/model/EdgeUse.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Manager.h"
#include "smtk/model/UseEntity.h"

namespace py = pybind11;

py::class_< smtk::model::EdgeUse, smtk::model::UseEntity > pybind11_init_smtk_model_EdgeUse(py::module &m)
{
  py::class_< smtk::model::EdgeUse, smtk::model::UseEntity > instance(m, "EdgeUse");
  instance
    .def(py::init<::smtk::model::EdgeUse const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ManagerPtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::EdgeUse::*)(::smtk::model::EntityRef const &) const) &smtk::model::EdgeUse::operator!=)
    .def("deepcopy", (smtk::model::EdgeUse & (smtk::model::EdgeUse::*)(::smtk::model::EdgeUse const &)) &smtk::model::EdgeUse::operator=)
    .def("__eq__", (bool (smtk::model::EdgeUse::*)(::smtk::model::EntityRef const &) const) &smtk::model::EdgeUse::operator==)
    .def("ccwUse", &smtk::model::EdgeUse::ccwUse)
    .def("chains", &smtk::model::EdgeUse::chains)
    .def("classname", &smtk::model::EdgeUse::classname)
    .def("cwUse", &smtk::model::EdgeUse::cwUse)
    .def("edge", &smtk::model::EdgeUse::edge)
    .def("faceUse", &smtk::model::EdgeUse::faceUse)
    .def("isValid", (bool (smtk::model::EdgeUse::*)() const) &smtk::model::EdgeUse::isValid)
    // .def("isValid", (bool (smtk::model::EdgeUse::*)(::smtk::model::Entity * *) const) &smtk::model::EdgeUse::isValid, py::arg("entRec"))
    .def("loop", &smtk::model::EdgeUse::loop)
    .def("vertexUses", &smtk::model::EdgeUse::vertexUses)
    .def("vertices", &smtk::model::EdgeUse::vertices)
    ;
  return instance;
}

#endif
