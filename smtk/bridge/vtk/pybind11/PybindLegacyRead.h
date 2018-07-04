//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_vtk_LegacyRead_h
#define pybind_smtk_bridge_vtk_LegacyRead_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/vtk/operators/LegacyRead.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::vtk::LegacyRead > pybind11_init_smtk_bridge_vtk_LegacyRead(py::module &m, PySharedPtrClass< smtk::bridge::vtk::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::bridge::vtk::LegacyRead > instance(m, "LegacyRead", parent);
  instance
    .def(py::init<>())
    .def("deepcopy", (smtk::bridge::vtk::LegacyRead & (smtk::bridge::vtk::LegacyRead::*)(::smtk::bridge::vtk::LegacyRead const &)) &smtk::bridge::vtk::LegacyRead::operator=)
    .def_static("create", (std::shared_ptr<smtk::bridge::vtk::LegacyRead> (*)()) &smtk::bridge::vtk::LegacyRead::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::vtk::LegacyRead> (*)(::std::shared_ptr<smtk::bridge::vtk::LegacyRead> &)) &smtk::bridge::vtk::LegacyRead::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::vtk::LegacyRead> (smtk::bridge::vtk::LegacyRead::*)() const) &smtk::bridge::vtk::LegacyRead::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::vtk::LegacyRead> (smtk::bridge::vtk::LegacyRead::*)()) &smtk::bridge::vtk::LegacyRead::shared_from_this)
    ;
  return instance;
}

#endif
