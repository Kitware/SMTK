//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_CreateCylinder_h
#define pybind_smtk_bridge_cgm_operators_CreateCylinder_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/CreateCylinder.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::CreateCylinder > pybind11_init_smtk_bridge_cgm_CreateCylinder(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::CreateCylinder > instance(m, "CreateCylinder", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::CreateCylinder const &>())
    .def("deepcopy", (smtk::bridge::cgm::CreateCylinder & (smtk::bridge::cgm::CreateCylinder::*)(::smtk::bridge::cgm::CreateCylinder const &)) &smtk::bridge::cgm::CreateCylinder::operator=)
    .def_static("baseCreate", &smtk::bridge::cgm::CreateCylinder::baseCreate)
    .def("className", &smtk::bridge::cgm::CreateCylinder::className)
    .def("classname", &smtk::bridge::cgm::CreateCylinder::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::CreateCylinder> (*)()) &smtk::bridge::cgm::CreateCylinder::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::CreateCylinder> (*)(::std::shared_ptr<smtk::bridge::cgm::CreateCylinder> &)) &smtk::bridge::cgm::CreateCylinder::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::CreateCylinder::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::CreateCylinder> (smtk::bridge::cgm::CreateCylinder::*)() const) &smtk::bridge::cgm::CreateCylinder::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::CreateCylinder> (smtk::bridge::cgm::CreateCylinder::*)()) &smtk::bridge::cgm::CreateCylinder::shared_from_this)
    ;
  return instance;
}

#endif
