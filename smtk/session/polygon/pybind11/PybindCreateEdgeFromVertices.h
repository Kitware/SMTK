//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_operators_CreateEdgeFromVertices_h
#define pybind_smtk_session_polygon_operators_CreateEdgeFromVertices_h

#include <pybind11/pybind11.h>

#include "smtk/session/polygon/operators/CreateEdgeFromVertices.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::polygon::CreateEdgeFromVertices > pybind11_init_smtk_session_polygon_CreateEdgeFromVertices(py::module &m, PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::polygon::CreateEdgeFromVertices > instance(m, "CreateEdgeFromVertices", parent);
  instance
    .def_static("create", (std::shared_ptr<smtk::session::polygon::CreateEdgeFromVertices> (*)()) &smtk::session::polygon::CreateEdgeFromVertices::create)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::CreateEdgeFromVertices> (*)(::std::shared_ptr<smtk::session::polygon::CreateEdgeFromVertices> &)) &smtk::session::polygon::CreateEdgeFromVertices::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::polygon::CreateEdgeFromVertices> (smtk::session::polygon::CreateEdgeFromVertices::*)() const) &smtk::session::polygon::CreateEdgeFromVertices::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::polygon::CreateEdgeFromVertices> (smtk::session::polygon::CreateEdgeFromVertices::*)()) &smtk::session::polygon::CreateEdgeFromVertices::shared_from_this)
    ;
  return instance;
}

#endif
