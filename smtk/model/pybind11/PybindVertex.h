//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Vertex_h
#define pybind_smtk_model_Vertex_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/model/Vertex.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"

namespace py = pybind11;

py::class_< smtk::model::Vertex, smtk::model::CellEntity > pybind11_init_smtk_model_Vertex(py::module &m)
{
  py::class_< smtk::model::Vertex, smtk::model::CellEntity > instance(m, "Vertex");
  instance
    .def(py::init<::smtk::model::Vertex const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ManagerPtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::Vertex::*)(::smtk::model::EntityRef const &) const) &smtk::model::Vertex::operator!=)
    .def("deepcopy", (smtk::model::Vertex & (smtk::model::Vertex::*)(::smtk::model::Vertex const &)) &smtk::model::Vertex::operator=)
    .def("__eq__", (bool (smtk::model::Vertex::*)(::smtk::model::EntityRef const &) const) &smtk::model::Vertex::operator==)
    .def("classname", &smtk::model::Vertex::classname)
    // .def("coordinates", &smtk::model::Vertex::coordinates)
    .def("coordinates", [](const smtk::model::Vertex& v) {
        return std::vector<double>{v.coordinates()[0], v.coordinates()[1],
            v.coordinates()[2]};
      })
    .def("edges", &smtk::model::Vertex::edges)
    .def("isValid", (bool (smtk::model::Vertex::*)() const) &smtk::model::Vertex::isValid)
    // .def("isValid", (bool (smtk::model::Vertex::*)(::smtk::model::Entity * *) const) &smtk::model::Vertex::isValid, py::arg("entRec"))
    ;
  return instance;
}

#endif
