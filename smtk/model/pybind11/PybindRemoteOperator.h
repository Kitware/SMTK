//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_RemoteOperator_h
#define pybind_smtk_model_RemoteOperator_h

#include <pybind11/pybind11.h>

#include "smtk/model/RemoteOperator.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::RemoteOperator, smtk::model::Operator > pybind11_init_smtk_model_RemoteOperator(py::module &m)
{
  PySharedPtrClass< smtk::model::RemoteOperator, smtk::model::Operator > instance(m, "RemoteOperator", py::metaclass());
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::RemoteOperator const &>())
    .def("deepcopy", (smtk::model::RemoteOperator & (smtk::model::RemoteOperator::*)(::smtk::model::RemoteOperator const &)) &smtk::model::RemoteOperator::operator=)
    .def("ableToOperate", &smtk::model::RemoteOperator::ableToOperate)
    .def_static("baseCreate", &smtk::model::RemoteOperator::baseCreate)
    .def("className", &smtk::model::RemoteOperator::className)
    .def("classname", &smtk::model::RemoteOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::model::RemoteOperator> (*)()) &smtk::model::RemoteOperator::create)
    .def_static("create", (std::shared_ptr<smtk::model::RemoteOperator> (*)(::std::shared_ptr<smtk::model::RemoteOperator> &)) &smtk::model::RemoteOperator::create, py::arg("ref"))
    .def("name", &smtk::model::RemoteOperator::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::model::RemoteOperator> (smtk::model::RemoteOperator::*)() const) &smtk::model::RemoteOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::RemoteOperator> (smtk::model::RemoteOperator::*)()) &smtk::model::RemoteOperator::shared_from_this)
    .def_readwrite_static("operatorName", &smtk::model::RemoteOperator::operatorName)
    ;
  return instance;
}

#endif
