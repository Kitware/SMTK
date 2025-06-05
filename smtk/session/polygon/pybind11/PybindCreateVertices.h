//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_operators_CreateVertices_h
#define pybind_smtk_session_polygon_operators_CreateVertices_h

#include <pybind11/pybind11.h>

#include "smtk/session/polygon/operators/CreateVertices.h"

#include "smtk/session/polygon/Operation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::polygon::CreateVertices> pybind11_init_smtk_session_polygon_CreateVertices(py::module &m, PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::polygon::CreateVertices > instance(m, "CreateVertices", parent);
  instance
    .def_static("create", (std::shared_ptr<smtk::session::polygon::CreateVertices> (*)()) &smtk::session::polygon::CreateVertices::create)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::CreateVertices> (*)(::std::shared_ptr<smtk::session::polygon::CreateVertices> &)) &smtk::session::polygon::CreateVertices::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::polygon::CreateVertices> (smtk::session::polygon::CreateVertices::*)() const) &smtk::session::polygon::CreateVertices::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::polygon::CreateVertices> (smtk::session::polygon::CreateVertices::*)()) &smtk::session::polygon::CreateVertices::shared_from_this)
    ;
  return instance;
}

#endif
