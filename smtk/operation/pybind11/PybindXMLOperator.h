//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_XMLOperator_h
#define pybind_smtk_operation_XMLOperator_h

#include <pybind11/pybind11.h>

#include "smtk/operation/XMLOperator.h"

#include "smtk/operation/NewOp.h"

namespace py = pybind11;

PySharedPtrClass< smtk::operation::XMLOperator, smtk::operation::NewOp > pybind11_init_smtk_operation_XMLOperator(py::module &m)
{
  PySharedPtrClass< smtk::operation::XMLOperator, smtk::operation::NewOp > instance(m, "XMLOperator");
  instance
    .def("deepcopy", (smtk::operation::XMLOperator & (smtk::operation::XMLOperator::*)(::smtk::operation::XMLOperator const &)) &smtk::operation::XMLOperator::operator=)
    .def("classname", &smtk::operation::XMLOperator::classname)
    .def("shared_from_this", (std::shared_ptr<const smtk::operation::XMLOperator> (smtk::operation::XMLOperator::*)() const) &smtk::operation::XMLOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::operation::XMLOperator> (smtk::operation::XMLOperator::*)()) &smtk::operation::XMLOperator::shared_from_this)
    ;
  return instance;
}

#endif
