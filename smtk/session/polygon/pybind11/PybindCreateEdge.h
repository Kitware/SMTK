//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_operators_CreateEdge_h
#define pybind_smtk_session_polygon_operators_CreateEdge_h

#include <pybind11/pybind11.h>

#include "smtk/session/polygon/operators/CreateEdge.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::polygon::CreateEdge > pybind11_init_smtk_session_polygon_CreateEdge(py::module &m, PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::polygon::CreateEdge > instance(m, "CreateEdge", parent);
  instance
    .def_static("create", (std::shared_ptr<smtk::session::polygon::CreateEdge> (*)()) &smtk::session::polygon::CreateEdge::create)
    ;
  return instance;
}

#endif
