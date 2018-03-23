//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_LegacyRead_h
#define pybind_smtk_bridge_polygon_operators_LegacyRead_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/LegacyRead.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::LegacyRead > pybind11_init_smtk_bridge_polygon_LegacyRead(py::module &m, PySharedPtrClass< smtk::bridge::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::bridge::polygon::LegacyRead > instance(m, "LegacyRead", parent);
  instance
    .def(py::init<::smtk::bridge::polygon::LegacyRead const &>())
    .def("deepcopy", (smtk::bridge::polygon::LegacyRead & (smtk::bridge::polygon::LegacyRead::*)(::smtk::bridge::polygon::LegacyRead const &)) &smtk::bridge::polygon::LegacyRead::operator=)
    .def("ableToOperate", &smtk::bridge::polygon::LegacyRead::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::LegacyRead> (*)()) &smtk::bridge::polygon::LegacyRead::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::LegacyRead> (*)(::std::shared_ptr<smtk::bridge::polygon::LegacyRead> &)) &smtk::bridge::polygon::LegacyRead::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::LegacyRead> (smtk::bridge::polygon::LegacyRead::*)() const) &smtk::bridge::polygon::LegacyRead::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::LegacyRead> (smtk::bridge::polygon::LegacyRead::*)()) &smtk::bridge::polygon::LegacyRead::shared_from_this)
    ;

  m.def("legacyRead", (smtk::resource::ResourcePtr (*)(::std::string const &)) &smtk::bridge::polygon::legacyRead, "", py::arg("filePath"));

  return instance;
}

#endif
