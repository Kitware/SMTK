//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_CreateBrick_h
#define pybind_smtk_bridge_cgm_operators_CreateBrick_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/CreateBrick.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::CreateBrick > pybind11_init_smtk_bridge_cgm_CreateBrick(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::CreateBrick > instance(m, "CreateBrick", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::CreateBrick const &>())
    .def("deepcopy", (smtk::bridge::cgm::CreateBrick & (smtk::bridge::cgm::CreateBrick::*)(::smtk::bridge::cgm::CreateBrick const &)) &smtk::bridge::cgm::CreateBrick::operator=)
    .def_static("baseCreate", &smtk::bridge::cgm::CreateBrick::baseCreate)
    .def("className", &smtk::bridge::cgm::CreateBrick::className)
    .def("classname", &smtk::bridge::cgm::CreateBrick::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::CreateBrick> (*)()) &smtk::bridge::cgm::CreateBrick::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::CreateBrick> (*)(::std::shared_ptr<smtk::bridge::cgm::CreateBrick> &)) &smtk::bridge::cgm::CreateBrick::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::CreateBrick::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::CreateBrick> (smtk::bridge::cgm::CreateBrick::*)() const) &smtk::bridge::cgm::CreateBrick::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::CreateBrick> (smtk::bridge::cgm::CreateBrick::*)()) &smtk::bridge::cgm::CreateBrick::shared_from_this)
    ;
  return instance;
}

#endif
