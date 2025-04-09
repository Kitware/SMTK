//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_operators_ExtractContours_h
#define pybind_smtk_session_polygon_operators_ExtractContours_h

#include <pybind11/pybind11.h>

#include "smtk/session/polygon/operators/ExtractContours.h"

#include "smtk/session/polygon/Operation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::polygon::ExtractContours > pybind11_init_smtk_session_polygon_ExtractContours(py::module &m, PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::polygon::ExtractContours > instance(m, "ExtractContours", parent);
  instance
    .def(py::init<>())
    .def("ableToOperate", &smtk::session::polygon::ExtractContours::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::ExtractContours> (*)()) &smtk::session::polygon::ExtractContours::create)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::ExtractContours> (*)(::std::shared_ptr<smtk::session::polygon::ExtractContours> &)) &smtk::session::polygon::ExtractContours::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::polygon::ExtractContours> (smtk::session::polygon::ExtractContours::*)() const) &smtk::session::polygon::ExtractContours::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::polygon::ExtractContours> (smtk::session::polygon::ExtractContours::*)()) &smtk::session::polygon::ExtractContours::shared_from_this)
    ;
  return instance;
}

#endif
