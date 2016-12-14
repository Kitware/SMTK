//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_EntityGroupOperator_h
#define pybind_smtk_bridge_discrete_operators_EntityGroupOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/EntityGroupOperator.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::EntityGroupOperator, smtk::model::Operator > pybind11_init_smtk_bridge_discrete_EntityGroupOperator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::EntityGroupOperator, smtk::model::Operator > instance(m, "EntityGroupOperator");
  instance
    .def("classname", &smtk::bridge::discrete::EntityGroupOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::EntityGroupOperator> (*)()) &smtk::bridge::discrete::EntityGroupOperator::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::EntityGroupOperator> (*)(::std::shared_ptr<smtk::bridge::discrete::EntityGroupOperator> &)) &smtk::bridge::discrete::EntityGroupOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::EntityGroupOperator> (smtk::bridge::discrete::EntityGroupOperator::*)()) &smtk::bridge::discrete::EntityGroupOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::EntityGroupOperator> (smtk::bridge::discrete::EntityGroupOperator::*)() const) &smtk::bridge::discrete::EntityGroupOperator::shared_from_this)
    .def("name", &smtk::bridge::discrete::EntityGroupOperator::name)
    .def("className", &smtk::bridge::discrete::EntityGroupOperator::className)
    .def_static("baseCreate", &smtk::bridge::discrete::EntityGroupOperator::baseCreate)
    .def("ableToOperate", &smtk::bridge::discrete::EntityGroupOperator::ableToOperate)
    ;
  return instance;
}

#endif
