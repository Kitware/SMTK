//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_vtk_Read_h
#define pybind_smtk_session_vtk_Read_h

#include <pybind11/pybind11.h>

#include "smtk/session/vtk/operators/Read.h"

#include "smtk/common/Managers.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::vtk::Read > pybind11_init_smtk_session_vtk_Read(py::module &m, PySharedPtrClass< smtk::session::vtk::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::vtk::Read > instance(m, "Read", parent);
  instance
    .def(py::init<>())
    .def_static("create", (std::shared_ptr<smtk::session::vtk::Read> (*)()) &smtk::session::vtk::Read::create)
    .def_static("create", (std::shared_ptr<smtk::session::vtk::Read> (*)(::std::shared_ptr<smtk::session::vtk::Read> &)) &smtk::session::vtk::Read::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::vtk::Read> (smtk::session::vtk::Read::*)() const) &smtk::session::vtk::Read::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::vtk::Read> (smtk::session::vtk::Read::*)()) &smtk::session::vtk::Read::shared_from_this)
    ;

  m.def("read", (smtk::resource::ResourcePtr (*)(::std::string const &, const std::shared_ptr<smtk::common::Managers>&)) &smtk::session::vtk::read, "", py::arg("filePath"), py::arg("managers") = nullptr);

  return instance;
}

#endif
