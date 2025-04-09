//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_operators_Delete_h
#define pybind_smtk_session_polygon_operators_Delete_h

#include <pybind11/pybind11.h>

#include "smtk/session/polygon/operators/Delete.h"

#include "smtk/session/polygon/Operation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::polygon::Delete> pybind11_init_smtk_session_polygon_Delete(py::module &m, PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::polygon::Delete > instance(m, "Delete", parent);
  instance
    .def_static("create", (std::shared_ptr<smtk::session::polygon::Delete> (*)()) &smtk::session::polygon::Delete::create)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::Delete> (*)(::std::shared_ptr<smtk::session::polygon::Delete> &)) &smtk::session::polygon::Delete::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::polygon::Delete> (smtk::session::polygon::Delete::*)() const) &smtk::session::polygon::Delete::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::polygon::Delete> (smtk::session::polygon::Delete::*)()) &smtk::session::polygon::Delete::shared_from_this)
    ;
  return instance;
}

#endif
