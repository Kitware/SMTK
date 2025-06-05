//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_operators_SplitEdge_h
#define pybind_smtk_session_polygon_operators_SplitEdge_h

#include <pybind11/pybind11.h>

#include "smtk/session/polygon/operators/SplitEdge.h"

#include "smtk/session/polygon/Operation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::polygon::SplitEdge > pybind11_init_smtk_session_polygon_SplitEdge(py::module &m, PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::polygon::SplitEdge > instance(m, "SplitEdge", parent);
  instance
    .def_static("create", (std::shared_ptr<smtk::session::polygon::SplitEdge> (*)()) &smtk::session::polygon::SplitEdge::create)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::SplitEdge> (*)(::std::shared_ptr<smtk::session::polygon::SplitEdge> &)) &smtk::session::polygon::SplitEdge::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::polygon::SplitEdge> (smtk::session::polygon::SplitEdge::*)() const) &smtk::session::polygon::SplitEdge::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::polygon::SplitEdge> (smtk::session::polygon::SplitEdge::*)()) &smtk::session::polygon::SplitEdge::shared_from_this)
    ;
  return instance;
}

#endif
