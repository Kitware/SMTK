//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_discrete_operators_EntityGroupOperation_h
#define pybind_smtk_session_discrete_operators_EntityGroupOperation_h

#include <pybind11/pybind11.h>

#include "smtk/session/discrete/operators/EntityGroupOperation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::session::discrete::EntityGroupOperation, smtk::operation::Operation > pybind11_init_smtk_session_discrete_EntityGroupOperation(py::module &m)
{
  PySharedPtrClass< smtk::session::discrete::EntityGroupOperation, smtk::operation::Operation > instance(m, "EntityGroupOperation");
  instance
    .def_static("create", (std::shared_ptr<smtk::session::discrete::EntityGroupOperation> (*)()) &smtk::session::discrete::EntityGroupOperation::create)
    .def_static("create", (std::shared_ptr<smtk::session::discrete::EntityGroupOperation> (*)(::std::shared_ptr<smtk::session::discrete::EntityGroupOperation> &)) &smtk::session::discrete::EntityGroupOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::discrete::EntityGroupOperation> (smtk::session::discrete::EntityGroupOperation::*)()) &smtk::session::discrete::EntityGroupOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::discrete::EntityGroupOperation> (smtk::session::discrete::EntityGroupOperation::*)() const) &smtk::session::discrete::EntityGroupOperator::shared_from_this)
    .def("name", &smtk::session::discrete::EntityGroupOperation::name)
    .def("ableToOperate", &smtk::session::discrete::EntityGroupOperation::ableToOperate)
    ;
  return instance;
}

#endif
