//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_operators_CreateEdgeFromPoints_h
#define pybind_smtk_session_polygon_operators_CreateEdgeFromPoints_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/Attribute.h"

#include "smtk/session/polygon/operators/CreateEdgeFromPoints.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::polygon::CreateEdgeFromPoints > pybind11_init_smtk_session_polygon_CreateEdgeFromPoints(py::module &m, PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::polygon::CreateEdgeFromPoints > instance(m, "CreateEdgeFromPoints", parent);
  instance
    .def_static("create", (std::shared_ptr<smtk::session::polygon::CreateEdgeFromPoints> (*)()) &smtk::session::polygon::CreateEdgeFromPoints::create)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::CreateEdgeFromPoints> (*)(::std::shared_ptr<smtk::session::polygon::CreateEdgeFromPoints> &)) &smtk::session::polygon::CreateEdgeFromPoints::create, py::arg("ref"))
    .def("process", &smtk::session::polygon::CreateEdgeFromPoints::process, py::arg("pnts"), py::arg("numCoordsPerPoint"), py::arg("parentModel"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::polygon::CreateEdgeFromPoints> (smtk::session::polygon::CreateEdgeFromPoints::*)() const) &smtk::session::polygon::CreateEdgeFromPoints::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::polygon::CreateEdgeFromPoints> (smtk::session::polygon::CreateEdgeFromPoints::*)()) &smtk::session::polygon::CreateEdgeFromPoints::shared_from_this)
    ;
  return instance;
}

#endif
