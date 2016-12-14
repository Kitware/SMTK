//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_ReadOperator_h
#define pybind_smtk_bridge_discrete_operators_ReadOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/ReadOperator.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::ReadOperator, smtk::model::Operator > pybind11_init_smtk_bridge_discrete_ReadOperator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::ReadOperator, smtk::model::Operator > instance(m, "ReadOperator");
  instance
    .def("classname", &smtk::bridge::discrete::ReadOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::ReadOperator> (*)()) &smtk::bridge::discrete::ReadOperator::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::ReadOperator> (*)(::std::shared_ptr<smtk::bridge::discrete::ReadOperator> &)) &smtk::bridge::discrete::ReadOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::ReadOperator> (smtk::bridge::discrete::ReadOperator::*)()) &smtk::bridge::discrete::ReadOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::ReadOperator> (smtk::bridge::discrete::ReadOperator::*)() const) &smtk::bridge::discrete::ReadOperator::shared_from_this)
    .def("name", &smtk::bridge::discrete::ReadOperator::name)
    .def("className", &smtk::bridge::discrete::ReadOperator::className)
    .def_static("baseCreate", &smtk::bridge::discrete::ReadOperator::baseCreate)
    .def("ableToOperate", &smtk::bridge::discrete::ReadOperator::ableToOperate)
    ;
  return instance;
}

#endif
