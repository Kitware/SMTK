//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_Sweep_h
#define pybind_smtk_bridge_cgm_operators_Sweep_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/Sweep.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::Sweep > pybind11_init_smtk_bridge_cgm_Sweep(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::Sweep > instance(m, "Sweep", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::Sweep const &>())
    .def("deepcopy", (smtk::bridge::cgm::Sweep & (smtk::bridge::cgm::Sweep::*)(::smtk::bridge::cgm::Sweep const &)) &smtk::bridge::cgm::Sweep::operator=)
    .def_static("baseCreate", &smtk::bridge::cgm::Sweep::baseCreate)
    .def("className", &smtk::bridge::cgm::Sweep::className)
    .def("classname", &smtk::bridge::cgm::Sweep::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Sweep> (*)()) &smtk::bridge::cgm::Sweep::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Sweep> (*)(::std::shared_ptr<smtk::bridge::cgm::Sweep> &)) &smtk::bridge::cgm::Sweep::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::Sweep::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::Sweep> (smtk::bridge::cgm::Sweep::*)() const) &smtk::bridge::cgm::Sweep::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::Sweep> (smtk::bridge::cgm::Sweep::*)()) &smtk::bridge::cgm::Sweep::shared_from_this)
    ;
  return instance;
}

#endif
