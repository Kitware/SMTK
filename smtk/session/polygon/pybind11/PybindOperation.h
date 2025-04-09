//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_Operation_h
#define pybind_smtk_session_polygon_Operation_h

#include <pybind11/pybind11.h>

#include "smtk/session/polygon/Operation.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation > pybind11_init_smtk_session_polygon_Operation(py::module &m)
{
  PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation > instance(m, "Operation");
  return instance;
}

#endif
