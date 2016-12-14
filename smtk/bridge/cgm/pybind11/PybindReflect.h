//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_Reflect_h
#define pybind_smtk_bridge_cgm_operators_Reflect_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/Reflect.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::Reflect > pybind11_init_smtk_bridge_cgm_Reflect(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::Reflect > instance(m, "Reflect", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::Reflect const &>())
    .def("deepcopy", (smtk::bridge::cgm::Reflect & (smtk::bridge::cgm::Reflect::*)(::smtk::bridge::cgm::Reflect const &)) &smtk::bridge::cgm::Reflect::operator=)
    .def_static("baseCreate", &smtk::bridge::cgm::Reflect::baseCreate)
    .def("className", &smtk::bridge::cgm::Reflect::className)
    .def("classname", &smtk::bridge::cgm::Reflect::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Reflect> (*)()) &smtk::bridge::cgm::Reflect::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Reflect> (*)(::std::shared_ptr<smtk::bridge::cgm::Reflect> &)) &smtk::bridge::cgm::Reflect::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::Reflect::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::Reflect> (smtk::bridge::cgm::Reflect::*)() const) &smtk::bridge::cgm::Reflect::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::Reflect> (smtk::bridge::cgm::Reflect::*)()) &smtk::bridge::cgm::Reflect::shared_from_this)
    ;
  return instance;
}

#endif
