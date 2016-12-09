//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_VolumeUse_h
#define pybind_smtk_model_VolumeUse_h

#include <pybind11/pybind11.h>

#include "smtk/model/VolumeUse.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"
#include "smtk/model/UseEntity.h"
#include "smtk/model/Volume.h"

namespace py = pybind11;

py::class_< smtk::model::VolumeUse, smtk::model::UseEntity > pybind11_init_smtk_model_VolumeUse(py::module &m)
{
  py::class_< smtk::model::VolumeUse, smtk::model::UseEntity > instance(m, "VolumeUse");
  instance
    .def(py::init<::smtk::model::VolumeUse const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ManagerPtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::VolumeUse::*)(::smtk::model::EntityRef const &) const) &smtk::model::VolumeUse::operator!=)
    .def("deepcopy", (smtk::model::VolumeUse & (smtk::model::VolumeUse::*)(::smtk::model::VolumeUse const &)) &smtk::model::VolumeUse::operator=)
    .def("__eq__", (bool (smtk::model::VolumeUse::*)(::smtk::model::EntityRef const &) const) &smtk::model::VolumeUse::operator==)
    .def("classname", &smtk::model::VolumeUse::classname)
    .def("isValid", (bool (smtk::model::VolumeUse::*)() const) &smtk::model::VolumeUse::isValid)
    // .def("isValid", (bool (smtk::model::VolumeUse::*)(::smtk::model::Entity * *) const) &smtk::model::VolumeUse::isValid, py::arg("entRec"))
    .def("shells", &smtk::model::VolumeUse::shells)
    .def("volume", &smtk::model::VolumeUse::volume)
    ;
  return instance;
}

#endif
