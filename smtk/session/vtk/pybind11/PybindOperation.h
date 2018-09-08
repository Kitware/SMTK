//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_vtk_Operation_h
#define pybind_smtk_session_vtk_Operation_h

#include <pybind11/pybind11.h>

#include "smtk/session/vtk/Operation.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::vtk::Operation, smtk::operation::XMLOperation > pybind11_init_smtk_session_vtk_Operation(py::module &m)
{
  PySharedPtrClass< smtk::session::vtk::Operation, smtk::operation::XMLOperation > instance(m, "Operation");
  instance
    .def("deepcopy", (smtk::session::vtk::Operation & (smtk::session::vtk::Operation::*)(::smtk::session::vtk::Operation const &)) &smtk::session::vtk::Operation::operator=)
    ;
  return instance;
}

#endif
