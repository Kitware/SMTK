//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_Write_h
#define pybind_smtk_bridge_polygon_operators_Write_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/Write.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::Write > pybind11_init_smtk_bridge_polygon_Write(py::module &m, PySharedPtrClass< smtk::bridge::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::bridge::polygon::Write > instance(m, "Write", parent);
  instance
    .def(py::init<::smtk::bridge::polygon::Write const &>())
    .def("deepcopy", (smtk::bridge::polygon::Write & (smtk::bridge::polygon::Write::*)(::smtk::bridge::polygon::Write const &)) &smtk::bridge::polygon::Write::operator=)
    .def("ableToOperate", &smtk::bridge::polygon::Write::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::Write> (*)()) &smtk::bridge::polygon::Write::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::Write> (*)(::std::shared_ptr<smtk::bridge::polygon::Write> &)) &smtk::bridge::polygon::Write::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::Write> (smtk::bridge::polygon::Write::*)() const) &smtk::bridge::polygon::Write::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::Write> (smtk::bridge::polygon::Write::*)()) &smtk::bridge::polygon::Write::shared_from_this)
    ;

  m.def("write", (bool (*)(const smtk::resource::ResourcePtr)) &smtk::bridge::polygon::write, "", py::arg("resource"));

  return instance;
}

#endif
