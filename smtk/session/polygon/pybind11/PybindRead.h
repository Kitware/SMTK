//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_operators_Read_h
#define pybind_smtk_session_polygon_operators_Read_h

#include <pybind11/pybind11.h>

#include "smtk/common/Managers.h"

#include "smtk/session/polygon/operators/Read.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::polygon::Read > pybind11_init_smtk_session_polygon_Read(py::module &m, PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::polygon::Read > instance(m, "Read", parent);
  instance
    .def("ableToOperate", &smtk::session::polygon::Read::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::Read> (*)()) &smtk::session::polygon::Read::create)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::Read> (*)(::std::shared_ptr<smtk::session::polygon::Read> &)) &smtk::session::polygon::Read::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::polygon::Read> (smtk::session::polygon::Read::*)() const) &smtk::session::polygon::Read::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::polygon::Read> (smtk::session::polygon::Read::*)()) &smtk::session::polygon::Read::shared_from_this)
    ;

  m.def("read", (smtk::resource::ResourcePtr (*)(::std::string const &, const std::shared_ptr<smtk::common::Managers>&)) &smtk::session::polygon::read, "", py::arg("filePath"), py::arg("managers") = nullptr);

  return instance;
}

#endif
