//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_BooleanSubtraction_h
#define pybind_smtk_bridge_cgm_operators_BooleanSubtraction_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/BooleanSubtraction.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::BooleanSubtraction > pybind11_init_smtk_bridge_cgm_BooleanSubtraction(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::BooleanSubtraction > instance(m, "BooleanSubtraction", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::BooleanSubtraction const &>())
    .def("deepcopy", (smtk::bridge::cgm::BooleanSubtraction & (smtk::bridge::cgm::BooleanSubtraction::*)(::smtk::bridge::cgm::BooleanSubtraction const &)) &smtk::bridge::cgm::BooleanSubtraction::operator=)
    .def_static("baseCreate", &smtk::bridge::cgm::BooleanSubtraction::baseCreate)
    .def("className", &smtk::bridge::cgm::BooleanSubtraction::className)
    .def("classname", &smtk::bridge::cgm::BooleanSubtraction::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::BooleanSubtraction> (*)()) &smtk::bridge::cgm::BooleanSubtraction::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::BooleanSubtraction> (*)(::std::shared_ptr<smtk::bridge::cgm::BooleanSubtraction> &)) &smtk::bridge::cgm::BooleanSubtraction::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::BooleanSubtraction::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::BooleanSubtraction> (smtk::bridge::cgm::BooleanSubtraction::*)() const) &smtk::bridge::cgm::BooleanSubtraction::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::BooleanSubtraction> (smtk::bridge::cgm::BooleanSubtraction::*)()) &smtk::bridge::cgm::BooleanSubtraction::shared_from_this)
    ;
  return instance;
}

#endif
