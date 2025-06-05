//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_operators_CreateModel_h
#define pybind_smtk_session_polygon_operators_CreateModel_h

#include <pybind11/pybind11.h>

#include "smtk/session/polygon/operators/CreateModel.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::polygon::CreateModel > pybind11_init_smtk_session_polygon_CreateModel(py::module &m, PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::polygon::CreateModel > instance(m, "CreateModel", parent);
  instance
    .def_static("create", (std::shared_ptr<smtk::session::polygon::CreateModel> (*)()) &smtk::session::polygon::CreateModel::create)
    ;
  return instance;
}

#endif
