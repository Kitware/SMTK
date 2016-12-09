//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_EntityRefArrangementOps_h
#define pybind_smtk_model_EntityRefArrangementOps_h

#include <pybind11/pybind11.h>

#include "smtk/model/EntityRefArrangementOps.h"

#include "smtk/model/ArrangementKind.h"
#include "smtk/model/EntityRef.h"

namespace py = pybind11;

py::class_< smtk::model::EntityRefArrangementOps > pybind11_init_smtk_model_EntityRefArrangementOps(py::module &m)
{
  py::class_< smtk::model::EntityRefArrangementOps > instance(m, "EntityRefArrangementOps");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRefArrangementOps const &>())
    .def("deepcopy", (smtk::model::EntityRefArrangementOps & (smtk::model::EntityRefArrangementOps::*)(::smtk::model::EntityRefArrangementOps const &)) &smtk::model::EntityRefArrangementOps::operator=)
    .def_static("findOrAddSimpleRelationship", &smtk::model::EntityRefArrangementOps::findOrAddSimpleRelationship, py::arg("a"), py::arg("k"), py::arg("b"))
    .def_static("findSimpleRelationship", &smtk::model::EntityRefArrangementOps::findSimpleRelationship, py::arg("a"), py::arg("k"), py::arg("b"))
    ;
  return instance;
}

#endif
