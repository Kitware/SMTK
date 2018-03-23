//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_Read_h
#define pybind_smtk_bridge_polygon_operators_Read_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/Read.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::Read > pybind11_init_smtk_bridge_polygon_Read(py::module &m, PySharedPtrClass< smtk::bridge::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::bridge::polygon::Read > instance(m, "Read", parent);
  instance
    .def(py::init<::smtk::bridge::polygon::Read const &>())
    .def("deepcopy", (smtk::bridge::polygon::Read & (smtk::bridge::polygon::Read::*)(::smtk::bridge::polygon::Read const &)) &smtk::bridge::polygon::Read::operator=)
    .def("ableToOperate", &smtk::bridge::polygon::Read::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::Read> (*)()) &smtk::bridge::polygon::Read::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::Read> (*)(::std::shared_ptr<smtk::bridge::polygon::Read> &)) &smtk::bridge::polygon::Read::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::Read> (smtk::bridge::polygon::Read::*)() const) &smtk::bridge::polygon::Read::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::Read> (smtk::bridge::polygon::Read::*)()) &smtk::bridge::polygon::Read::shared_from_this)
    ;

  m.def("read", (smtk::resource::ResourcePtr (*)(::std::string const &)) &smtk::bridge::polygon::read, "", py::arg("filePath"));

  return instance;
}

#endif
