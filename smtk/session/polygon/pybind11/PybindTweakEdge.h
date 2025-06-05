//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_operators_TweakEdge_h
#define pybind_smtk_session_polygon_operators_TweakEdge_h

#include <pybind11/pybind11.h>

#include "smtk/session/polygon/operators/TweakEdge.h"

#include "smtk/session/polygon/Operation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::polygon::TweakEdge > pybind11_init_smtk_session_polygon_TweakEdge(py::module &m, PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::polygon::TweakEdge > instance(m, "TweakEdge", parent);
  instance
    .def_static("create", (std::shared_ptr<smtk::session::polygon::TweakEdge> (*)()) &smtk::session::polygon::TweakEdge::create)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::TweakEdge> (*)(::std::shared_ptr<smtk::session::polygon::TweakEdge> &)) &smtk::session::polygon::TweakEdge::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::polygon::TweakEdge> (smtk::session::polygon::TweakEdge::*)() const) &smtk::session::polygon::TweakEdge::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::polygon::TweakEdge> (smtk::session::polygon::TweakEdge::*)()) &smtk::session::polygon::TweakEdge::shared_from_this)
    ;
  return instance;
}

#endif
