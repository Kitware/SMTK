//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_Operation_h
#define pybind_smtk_bridge_polygon_Operation_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/Operation.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::Operation, smtk::operation::XMLOperation > pybind11_init_smtk_bridge_polygon_Operation(py::module &m)
{
  PySharedPtrClass< smtk::bridge::polygon::Operation, smtk::operation::XMLOperation > instance(m, "Operation");
  instance
    .def("deepcopy", (smtk::bridge::polygon::Operation & (smtk::bridge::polygon::Operation::*)(::smtk::bridge::polygon::Operation const &)) &smtk::bridge::polygon::Operation::operator=)
    ;
  return instance;
}

#endif
