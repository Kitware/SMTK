//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_Translate_h
#define pybind_smtk_bridge_cgm_operators_Translate_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/Translate.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::Translate > pybind11_init_smtk_bridge_cgm_Translate(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::Translate > instance(m, "Translate", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::Translate const &>())
    .def("deepcopy", (smtk::bridge::cgm::Translate & (smtk::bridge::cgm::Translate::*)(::smtk::bridge::cgm::Translate const &)) &smtk::bridge::cgm::Translate::operator=)
    .def_static("baseCreate", &smtk::bridge::cgm::Translate::baseCreate)
    .def("className", &smtk::bridge::cgm::Translate::className)
    .def("classname", &smtk::bridge::cgm::Translate::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Translate> (*)()) &smtk::bridge::cgm::Translate::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Translate> (*)(::std::shared_ptr<smtk::bridge::cgm::Translate> &)) &smtk::bridge::cgm::Translate::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::Translate::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::Translate> (smtk::bridge::cgm::Translate::*)() const) &smtk::bridge::cgm::Translate::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::Translate> (smtk::bridge::cgm::Translate::*)()) &smtk::bridge::cgm::Translate::shared_from_this)
    ;
  return instance;
}

#endif
