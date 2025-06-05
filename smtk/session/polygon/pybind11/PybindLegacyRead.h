//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_operators_LegacyRead_h
#define pybind_smtk_session_polygon_operators_LegacyRead_h

#include <pybind11/pybind11.h>

#include "smtk/session/polygon/operators/LegacyRead.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::polygon::LegacyRead > pybind11_init_smtk_session_polygon_LegacyRead(py::module &m, PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::polygon::LegacyRead > instance(m, "LegacyRead", parent);
  instance
    .def("ableToOperate", &smtk::session::polygon::LegacyRead::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::LegacyRead> (*)()) &smtk::session::polygon::LegacyRead::create)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::LegacyRead> (*)(::std::shared_ptr<smtk::session::polygon::LegacyRead> &)) &smtk::session::polygon::LegacyRead::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::polygon::LegacyRead> (smtk::session::polygon::LegacyRead::*)() const) &smtk::session::polygon::LegacyRead::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::polygon::LegacyRead> (smtk::session::polygon::LegacyRead::*)()) &smtk::session::polygon::LegacyRead::shared_from_this)
    ;

  m.def("legacyRead", (smtk::resource::ResourcePtr (*)(::std::string const &)) &smtk::session::polygon::legacyRead, "", py::arg("filePath"));

  return instance;
}

#endif
