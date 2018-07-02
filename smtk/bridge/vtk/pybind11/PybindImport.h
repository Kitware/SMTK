//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_vtk_Import_h
#define pybind_smtk_bridge_vtk_Import_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/vtk/operators/Import.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::vtk::Import > pybind11_init_smtk_bridge_vtk_Import(py::module &m, PySharedPtrClass< smtk::bridge::vtk::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::bridge::vtk::Import > instance(m, "Import", parent);
  instance
    .def(py::init<>())
    .def("deepcopy", (smtk::bridge::vtk::Import & (smtk::bridge::vtk::Import::*)(::smtk::bridge::vtk::Import const &)) &smtk::bridge::vtk::Import::operator=)
    .def_static("create", (std::shared_ptr<smtk::bridge::vtk::Import> (*)()) &smtk::bridge::vtk::Import::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::vtk::Import> (*)(::std::shared_ptr<smtk::bridge::vtk::Import> &)) &smtk::bridge::vtk::Import::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::vtk::Import> (smtk::bridge::vtk::Import::*)() const) &smtk::bridge::vtk::Import::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::vtk::Import> (smtk::bridge::vtk::Import::*)()) &smtk::bridge::vtk::Import::shared_from_this)
    ;

  m.def("importResource", (smtk::resource::ResourcePtr (*)(::std::string const &)) &smtk::bridge::vtk::importResource, "", py::arg("filePath"));

  return instance;
}

#endif
