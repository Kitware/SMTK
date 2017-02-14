//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Edge_h
#define pybind_smtk_model_Edge_h

#include <pybind11/pybind11.h>

#include "smtk/model/Edge.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"

namespace py = pybind11;

py::class_< smtk::model::Edge, smtk::model::CellEntity > pybind11_init_smtk_model_Edge(py::module &m)
{
  py::class_< smtk::model::Edge, smtk::model::CellEntity > instance(m, "Edge");
  instance
    .def(py::init<::smtk::model::Edge const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ManagerPtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::Edge::*)(::smtk::model::EntityRef const &) const) &smtk::model::Edge::operator!=)
    .def("deepcopy", (smtk::model::Edge & (smtk::model::Edge::*)(::smtk::model::Edge const &)) &smtk::model::Edge::operator=)
    .def("__eq__", (bool (smtk::model::Edge::*)(::smtk::model::EntityRef const &) const) &smtk::model::Edge::operator==)
    .def("classname", &smtk::model::Edge::classname)
    .def("edgeUses", &smtk::model::Edge::edgeUses)
    .def("faces", &smtk::model::Edge::faces)
    .def("findOrAddEdgeUse", &smtk::model::Edge::findOrAddEdgeUse, py::arg("o"), py::arg("sense") = 0)
    .def("isPeriodic", (bool (smtk::model::Edge::*)() const) &smtk::model::Edge::isPeriodic)
    .def("isValid", (bool (smtk::model::Edge::*)() const) &smtk::model::Edge::isValid)
    // .def("isValid", (bool (smtk::model::Edge::*)(::smtk::model::Entity * *) const) &smtk::model::Edge::isValid, py::arg("entRec"))
    .def("vertices", &smtk::model::Edge::vertices)
    ;
  return instance;
}

#endif
