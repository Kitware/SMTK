//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_vtk_Read_h
#define pybind_smtk_bridge_vtk_Read_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/vtk/operators/Read.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::vtk::Read > pybind11_init_smtk_bridge_vtk_Read(py::module &m, PySharedPtrClass< smtk::bridge::vtk::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::bridge::vtk::Read > instance(m, "Read", parent);
  instance
    .def(py::init<>())
    .def("deepcopy", (smtk::bridge::vtk::Read & (smtk::bridge::vtk::Read::*)(::smtk::bridge::vtk::Read const &)) &smtk::bridge::vtk::Read::operator=)
    .def_static("create", (std::shared_ptr<smtk::bridge::vtk::Read> (*)()) &smtk::bridge::vtk::Read::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::vtk::Read> (*)(::std::shared_ptr<smtk::bridge::vtk::Read> &)) &smtk::bridge::vtk::Read::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::vtk::Read> (smtk::bridge::vtk::Read::*)() const) &smtk::bridge::vtk::Read::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::vtk::Read> (smtk::bridge::vtk::Read::*)()) &smtk::bridge::vtk::Read::shared_from_this)
    ;

  m.def("read", (smtk::resource::ResourcePtr (*)(::std::string const &)) &smtk::bridge::vtk::read, "", py::arg("filePath"));

  return instance;
}

#endif
