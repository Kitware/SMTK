//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Volume_h
#define pybind_smtk_model_Volume_h

#include <pybind11/pybind11.h>

#include "smtk/model/Volume.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"
#include "smtk/model/VolumeUse.h"

namespace py = pybind11;

py::class_< smtk::model::Volume, smtk::model::CellEntity > pybind11_init_smtk_model_Volume(py::module &m)
{
  py::class_< smtk::model::Volume, smtk::model::CellEntity > instance(m, "Volume");
  instance
    .def(py::init<::smtk::model::Volume const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ManagerPtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::Volume::*)(::smtk::model::EntityRef const &) const) &smtk::model::Volume::operator!=)
    .def("deepcopy", (smtk::model::Volume & (smtk::model::Volume::*)(::smtk::model::Volume const &)) &smtk::model::Volume::operator=)
    .def("__eq__", (bool (smtk::model::Volume::*)(::smtk::model::EntityRef const &) const) &smtk::model::Volume::operator==)
    .def("classname", &smtk::model::Volume::classname)
    .def("faces", &smtk::model::Volume::faces)
    .def("isValid", (bool (smtk::model::Volume::*)() const) &smtk::model::Volume::isValid)
    // .def("isValid", (bool (smtk::model::Volume::*)(::smtk::model::Entity * *) const) &smtk::model::Volume::isValid, py::arg("entRec"))
    .def("setVolumeUse", &smtk::model::Volume::setVolumeUse, py::arg("volUse"))
    .def("shells", &smtk::model::Volume::shells)
    .def("use", &smtk::model::Volume::use)
    ;
  return instance;
}

#endif
