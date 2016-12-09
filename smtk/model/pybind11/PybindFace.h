//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Face_h
#define pybind_smtk_model_Face_h

#include <pybind11/pybind11.h>

#include "smtk/model/Face.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Manager.h"

namespace py = pybind11;

py::class_< smtk::model::Face, smtk::model::CellEntity > pybind11_init_smtk_model_Face(py::module &m)
{
  py::class_< smtk::model::Face, smtk::model::CellEntity > instance(m, "Face");
  instance
    .def(py::init<::smtk::model::Face const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ManagerPtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::Face::*)(::smtk::model::EntityRef const &) const) &smtk::model::Face::operator!=)
    .def("deepcopy", (smtk::model::Face & (smtk::model::Face::*)(::smtk::model::Face const &)) &smtk::model::Face::operator=)
    .def("__eq__", (bool (smtk::model::Face::*)(::smtk::model::EntityRef const &) const) &smtk::model::Face::operator==)
    .def("classname", &smtk::model::Face::classname)
    .def("edges", &smtk::model::Face::edges)
    .def("isValid", (bool (smtk::model::Face::*)() const) &smtk::model::Face::isValid)
    // .def("isValid", (bool (smtk::model::Face::*)(::smtk::model::Entity * *) const) &smtk::model::Face::isValid, py::arg("entRec"))
    .def("negativeUse", &smtk::model::Face::negativeUse)
    .def("positiveUse", &smtk::model::Face::positiveUse)
    .def("setFaceUse", &smtk::model::Face::setFaceUse, py::arg("o"), py::arg("u"))
    .def("volumes", &smtk::model::Face::volumes)
    ;
  return instance;
}

#endif
