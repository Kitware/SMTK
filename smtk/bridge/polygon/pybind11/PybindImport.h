//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_Import_h
#define pybind_smtk_bridge_polygon_operators_Import_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/Import.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::Import > pybind11_init_smtk_bridge_polygon_Import(py::module &m, PySharedPtrClass< smtk::bridge::polygon::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::polygon::Import > instance(m, "Import", parent);
  instance
    .def(py::init<::smtk::bridge::polygon::Import const &>())
    .def("deepcopy", (smtk::bridge::polygon::Import & (smtk::bridge::polygon::Import::*)(::smtk::bridge::polygon::Import const &)) &smtk::bridge::polygon::Import::operator=)
    .def("ableToOperate", &smtk::bridge::polygon::Import::ableToOperate)
    .def_static("baseCreate", &smtk::bridge::polygon::Import::baseCreate)
    .def("className", &smtk::bridge::polygon::Import::className)
    .def("classname", &smtk::bridge::polygon::Import::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::Import> (*)()) &smtk::bridge::polygon::Import::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::Import> (*)(::std::shared_ptr<smtk::bridge::polygon::Import> &)) &smtk::bridge::polygon::Import::create, py::arg("ref"))
    .def("name", &smtk::bridge::polygon::Import::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::Import> (smtk::bridge::polygon::Import::*)() const) &smtk::bridge::polygon::Import::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::Import> (smtk::bridge::polygon::Import::*)()) &smtk::bridge::polygon::Import::shared_from_this)
    ;
  return instance;
}

#endif
