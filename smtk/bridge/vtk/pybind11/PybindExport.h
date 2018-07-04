//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_vtk_Export_h
#define pybind_smtk_bridge_vtk_Export_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/vtk/operators/Export.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::vtk::Export > pybind11_init_smtk_bridge_vtk_Export(py::module &m, PySharedPtrClass< smtk::bridge::vtk::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::bridge::vtk::Export > instance(m, "Export", parent);
  instance
    .def(py::init<>())
    .def("deepcopy", (smtk::bridge::vtk::Export & (smtk::bridge::vtk::Export::*)(::smtk::bridge::vtk::Export const &)) &smtk::bridge::vtk::Export::operator=)
    .def_static("create", (std::shared_ptr<smtk::bridge::vtk::Export> (*)()) &smtk::bridge::vtk::Export::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::vtk::Export> (*)(::std::shared_ptr<smtk::bridge::vtk::Export> &)) &smtk::bridge::vtk::Export::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::vtk::Export> (smtk::bridge::vtk::Export::*)() const) &smtk::bridge::vtk::Export::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::vtk::Export> (smtk::bridge::vtk::Export::*)()) &smtk::bridge::vtk::Export::shared_from_this)
    ;

  m.def("exportResource", (bool (*)(const smtk::resource::ResourcePtr)) &smtk::bridge::vtk::exportResource, "", py::arg("resource"));

  return instance;
}

#endif
