//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Chain_h
#define pybind_smtk_model_Chain_h

#include <pybind11/pybind11.h>

#include "smtk/model/Chain.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ShellEntity.h"

namespace py = pybind11;

py::class_< smtk::model::Chain, smtk::model::ShellEntity > pybind11_init_smtk_model_Chain(py::module &m)
{
  py::class_< smtk::model::Chain, smtk::model::ShellEntity > instance(m, "Chain");
  instance
    .def(py::init<::smtk::model::Chain const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ManagerPtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::Chain::*)(::smtk::model::EntityRef const &) const) &smtk::model::Chain::operator!=)
    .def("deepcopy", (smtk::model::Chain & (smtk::model::Chain::*)(::smtk::model::Chain const &)) &smtk::model::Chain::operator=)
    .def("__eq__", (bool (smtk::model::Chain::*)(::smtk::model::EntityRef const &) const) &smtk::model::Chain::operator==)
    .def("classname", &smtk::model::Chain::classname)
    .def("containedChains", &smtk::model::Chain::containedChains)
    .def("containingChain", &smtk::model::Chain::containingChain)
    .def("edge", &smtk::model::Chain::edge)
    .def("isValid", (bool (smtk::model::Chain::*)() const) &smtk::model::Chain::isValid)
    // .def("isValid", (bool (smtk::model::Chain::*)(::smtk::model::Entity * *) const) &smtk::model::Chain::isValid, py::arg("entRec"))
    .def("vertexUses", &smtk::model::Chain::vertexUses)
    ;
  return instance;
}

#endif
