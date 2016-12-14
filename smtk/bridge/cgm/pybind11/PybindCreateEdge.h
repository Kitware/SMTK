//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_CreateEdge_h
#define pybind_smtk_bridge_cgm_operators_CreateEdge_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/CreateEdge.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::CreateEdge > pybind11_init_smtk_bridge_cgm_CreateEdge(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::CreateEdge > instance(m, "CreateEdge", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::CreateEdge const &>())
    .def("deepcopy", (smtk::bridge::cgm::CreateEdge & (smtk::bridge::cgm::CreateEdge::*)(::smtk::bridge::cgm::CreateEdge const &)) &smtk::bridge::cgm::CreateEdge::operator=)
    .def_static("baseCreate", &smtk::bridge::cgm::CreateEdge::baseCreate)
    .def("className", &smtk::bridge::cgm::CreateEdge::className)
    .def("classname", &smtk::bridge::cgm::CreateEdge::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::CreateEdge> (*)()) &smtk::bridge::cgm::CreateEdge::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::CreateEdge> (*)(::std::shared_ptr<smtk::bridge::cgm::CreateEdge> &)) &smtk::bridge::cgm::CreateEdge::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::CreateEdge::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::CreateEdge> (smtk::bridge::cgm::CreateEdge::*)() const) &smtk::bridge::cgm::CreateEdge::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::CreateEdge> (smtk::bridge::cgm::CreateEdge::*)()) &smtk::bridge::cgm::CreateEdge::shared_from_this)
    ;
  return instance;
}

#endif
