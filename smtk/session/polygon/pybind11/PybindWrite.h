//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_operators_Write_h
#define pybind_smtk_session_polygon_operators_Write_h

#include <pybind11/pybind11.h>

#include "smtk/common/Managers.h"

#include "smtk/session/polygon/operators/Write.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::polygon::Write > pybind11_init_smtk_session_polygon_Write(py::module &m, PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::polygon::Write > instance(m, "Write", parent);
  instance
    .def(py::init<::smtk::session::polygon::Write const &>())
    .def("deepcopy", (smtk::session::polygon::Write & (smtk::session::polygon::Write::*)(::smtk::session::polygon::Write const &)) &smtk::session::polygon::Write::operator=)
    .def("ableToOperate", &smtk::session::polygon::Write::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::Write> (*)()) &smtk::session::polygon::Write::create)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::Write> (*)(::std::shared_ptr<smtk::session::polygon::Write> &)) &smtk::session::polygon::Write::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::polygon::Write> (smtk::session::polygon::Write::*)() const) &smtk::session::polygon::Write::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::polygon::Write> (smtk::session::polygon::Write::*)()) &smtk::session::polygon::Write::shared_from_this)
    ;

  m.def("write", (bool (*)(const smtk::resource::ResourcePtr&, const std::shared_ptr<smtk::common::Managers>&)) &smtk::session::polygon::write, "", py::arg("resource"), py::arg("managers") = nullptr);

  return instance;
}

#endif
