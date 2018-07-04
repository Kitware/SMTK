//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_vtk_Write_h
#define pybind_smtk_bridge_vtk_Write_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/vtk/operators/Write.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::vtk::Write > pybind11_init_smtk_bridge_vtk_Write(py::module &m, PySharedPtrClass< smtk::bridge::vtk::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::bridge::vtk::Write > instance(m, "Write", parent);
  instance
    .def(py::init<>())
    .def("deepcopy", (smtk::bridge::vtk::Write & (smtk::bridge::vtk::Write::*)(::smtk::bridge::vtk::Write const &)) &smtk::bridge::vtk::Write::operator=)
    .def_static("create", (std::shared_ptr<smtk::bridge::vtk::Write> (*)()) &smtk::bridge::vtk::Write::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::vtk::Write> (*)(::std::shared_ptr<smtk::bridge::vtk::Write> &)) &smtk::bridge::vtk::Write::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::vtk::Write> (smtk::bridge::vtk::Write::*)() const) &smtk::bridge::vtk::Write::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::vtk::Write> (smtk::bridge::vtk::Write::*)()) &smtk::bridge::vtk::Write::shared_from_this)
    ;

  m.def("write", (bool (*)(const smtk::resource::ResourcePtr)) &smtk::bridge::vtk::write, "", py::arg("resource"));

  return instance;
}

#endif
