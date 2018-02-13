//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_Operation_h
#define pybind_smtk_bridge_cgm_Operation_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/Operation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::Operation, smtk::operation::Operation > pybind11_init_smtk_bridge_cgm_Operation(py::module &m)
{
  PySharedPtrClass< smtk::bridge::cgm::Operation, smtk::operation::Operation > instance(m, "Operation");
  instance
    .def("deepcopy", (smtk::bridge::cgm::Operation & (smtk::bridge::cgm::Operation::*)(::smtk::bridge::cgm::Operator const &)) &smtk::bridge::cgm::Operator::operator=)
    ;
  return instance;
}

#endif
