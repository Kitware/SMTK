//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_vtk_LegacyRead_h
#define pybind_smtk_session_vtk_LegacyRead_h

#include <pybind11/pybind11.h>

#include "smtk/session/vtk/operators/LegacyRead.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::vtk::LegacyRead > pybind11_init_smtk_session_vtk_LegacyRead(py::module &m, PySharedPtrClass< smtk::session::vtk::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::vtk::LegacyRead > instance(m, "LegacyRead", parent);
  instance
    .def(py::init<>())
    .def_static("create", (std::shared_ptr<smtk::session::vtk::LegacyRead> (*)()) &smtk::session::vtk::LegacyRead::create)
    .def_static("create", (std::shared_ptr<smtk::session::vtk::LegacyRead> (*)(::std::shared_ptr<smtk::session::vtk::LegacyRead> &)) &smtk::session::vtk::LegacyRead::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::vtk::LegacyRead> (smtk::session::vtk::LegacyRead::*)() const) &smtk::session::vtk::LegacyRead::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::vtk::LegacyRead> (smtk::session::vtk::LegacyRead::*)()) &smtk::session::vtk::LegacyRead::shared_from_this)
    ;
  return instance;
}

#endif
