//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_vtk_Import_h
#define pybind_smtk_session_vtk_Import_h

#include <pybind11/pybind11.h>

#include "smtk/session/vtk/operators/Import.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::vtk::Import > pybind11_init_smtk_session_vtk_Import(py::module &m, PySharedPtrClass< smtk::session::vtk::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::vtk::Import > instance(m, "Import", parent);
  instance
    .def(py::init<>())
    .def_static("create", (std::shared_ptr<smtk::session::vtk::Import> (*)()) &smtk::session::vtk::Import::create)
    .def_static("create", (std::shared_ptr<smtk::session::vtk::Import> (*)(::std::shared_ptr<smtk::session::vtk::Import> &)) &smtk::session::vtk::Import::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::vtk::Import> (smtk::session::vtk::Import::*)() const) &smtk::session::vtk::Import::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::vtk::Import> (smtk::session::vtk::Import::*)()) &smtk::session::vtk::Import::shared_from_this)
    ;

  m.def("importResource", (smtk::resource::ResourcePtr (*)(::std::string const &)) &smtk::session::vtk::importResource, "", py::arg("filePath"));

  return instance;
}

#endif
