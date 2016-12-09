//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_BooleanUnion_h
#define pybind_smtk_bridge_cgm_operators_BooleanUnion_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/BooleanUnion.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::BooleanUnion > pybind11_init_smtk_bridge_cgm_BooleanUnion(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::BooleanUnion > instance(m, "BooleanUnion", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::BooleanUnion const &>())
    .def("deepcopy", (smtk::bridge::cgm::BooleanUnion & (smtk::bridge::cgm::BooleanUnion::*)(::smtk::bridge::cgm::BooleanUnion const &)) &smtk::bridge::cgm::BooleanUnion::operator=)
    .def_static("baseCreate", &smtk::bridge::cgm::BooleanUnion::baseCreate)
    .def("className", &smtk::bridge::cgm::BooleanUnion::className)
    .def("classname", &smtk::bridge::cgm::BooleanUnion::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::BooleanUnion> (*)()) &smtk::bridge::cgm::BooleanUnion::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::BooleanUnion> (*)(::std::shared_ptr<smtk::bridge::cgm::BooleanUnion> &)) &smtk::bridge::cgm::BooleanUnion::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::BooleanUnion::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::BooleanUnion> (smtk::bridge::cgm::BooleanUnion::*)() const) &smtk::bridge::cgm::BooleanUnion::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::BooleanUnion> (smtk::bridge::cgm::BooleanUnion::*)()) &smtk::bridge::cgm::BooleanUnion::shared_from_this)
    ;
  return instance;
}

#endif
