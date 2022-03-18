//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_vtk_Write_h
#define pybind_smtk_session_vtk_Write_h

#include <pybind11/pybind11.h>

#include "smtk/session/vtk/operators/Write.h"

#include "smtk/common/Managers.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::vtk::Write > pybind11_init_smtk_session_vtk_Write(py::module &m, PySharedPtrClass< smtk::session::vtk::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::vtk::Write > instance(m, "Write", parent);
  instance
    .def(py::init<>())
    .def("deepcopy", (smtk::session::vtk::Write & (smtk::session::vtk::Write::*)(::smtk::session::vtk::Write const &)) &smtk::session::vtk::Write::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::vtk::Write> (*)()) &smtk::session::vtk::Write::create)
    .def_static("create", (std::shared_ptr<smtk::session::vtk::Write> (*)(::std::shared_ptr<smtk::session::vtk::Write> &)) &smtk::session::vtk::Write::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::vtk::Write> (smtk::session::vtk::Write::*)() const) &smtk::session::vtk::Write::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::vtk::Write> (smtk::session::vtk::Write::*)()) &smtk::session::vtk::Write::shared_from_this)
    ;

  m.def("write", (bool (*)(const smtk::resource::ResourcePtr&, const std::shared_ptr<smtk::common::Managers>&)) &smtk::session::vtk::write, "", py::arg("resource"), py::arg("managers") = nullptr);

  return instance;
}

#endif
