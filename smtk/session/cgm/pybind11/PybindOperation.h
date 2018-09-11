//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_Operation_h
#define pybind_smtk_session_cgm_Operation_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/Operation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation > pybind11_init_smtk_session_cgm_Operation(py::module &m)
{
  PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation > instance(m, "Operation");
  instance
    .def("deepcopy", (smtk::session::cgm::Operation & (smtk::session::cgm::Operation::*)(::smtk::session::cgm::Operator const &)) &smtk::session::cgm::Operator::operator=)
    ;
  return instance;
}

#endif
