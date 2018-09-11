//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_operators_CleanGeometry_h
#define pybind_smtk_session_polygon_operators_CleanGeometry_h

#include <pybind11/pybind11.h>

#include "smtk/session/polygon/operators/CleanGeometry.h"

#include "smtk/session/polygon/Operation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::polygon::CleanGeometry, smtk::session::polygon::Operation > pybind11_init_smtk_session_polygon_CleanGeometry(py::module &m)
{
  PySharedPtrClass< smtk::session::polygon::CleanGeometry, smtk::session::polygon::Operation > instance(m, "CleanGeometry");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::polygon::CleanGeometry const &>())
    .def("deepcopy", (smtk::session::polygon::CleanGeometry & (smtk::session::polygon::CleanGeometry::*)(::smtk::session::polygon::CleanGeometry const &)) &smtk::session::polygon::CleanGeometry::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::CleanGeometry> (*)()) &smtk::session::polygon::CleanGeometry::create)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::CleanGeometry> (*)(::std::shared_ptr<smtk::session::polygon::CleanGeometry> &)) &smtk::session::polygon::CleanGeometry::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::polygon::CleanGeometry> (smtk::session::polygon::CleanGeometry::*)() const) &smtk::session::polygon::CleanGeometry::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::polygon::CleanGeometry> (smtk::session::polygon::CleanGeometry::*)()) &smtk::session::polygon::CleanGeometry::shared_from_this)
    ;
  return instance;
}

#endif
