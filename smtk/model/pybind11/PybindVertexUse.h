//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_VertexUse_h
#define pybind_smtk_model_VertexUse_h

#include <pybind11/pybind11.h>

#include "smtk/model/VertexUse.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"
#include "smtk/model/UseEntity.h"
#include "smtk/model/Vertex.h"

namespace py = pybind11;

py::class_< smtk::model::VertexUse, smtk::model::UseEntity > pybind11_init_smtk_model_VertexUse(py::module &m)
{
  py::class_< smtk::model::VertexUse, smtk::model::UseEntity > instance(m, "VertexUse");
  instance
    .def(py::init<::smtk::model::VertexUse const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ManagerPtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::VertexUse::*)(::smtk::model::EntityRef const &) const) &smtk::model::VertexUse::operator!=)
    .def("deepcopy", (smtk::model::VertexUse & (smtk::model::VertexUse::*)(::smtk::model::VertexUse const &)) &smtk::model::VertexUse::operator=)
    .def("__eq__", (bool (smtk::model::VertexUse::*)(::smtk::model::EntityRef const &) const) &smtk::model::VertexUse::operator==)
    .def("chains", &smtk::model::VertexUse::chains)
    .def("classname", &smtk::model::VertexUse::classname)
    .def("edges", &smtk::model::VertexUse::edges)
    .def("isValid", (bool (smtk::model::VertexUse::*)() const) &smtk::model::VertexUse::isValid)
    // .def("isValid", (bool (smtk::model::VertexUse::*)(::smtk::model::Entity * *) const) &smtk::model::VertexUse::isValid, py::arg("entRec"))
    .def("vertex", &smtk::model::VertexUse::vertex)
    ;
  return instance;
}

#endif
