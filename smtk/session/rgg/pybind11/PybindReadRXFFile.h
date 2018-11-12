//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_rgg_operators_ReadRXFFile_h
#define pybind_smtk_session_rgg_operators_ReadRXFFile_h

#include <pybind11/pybind11.h>

#include "smtk/session/rgg/operators/ReadRXFFile.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::rgg::ReadRXFFile, smtk::operation::XMLOperation > pybind11_init_smtk_session_rgg_ReadRXFFile(py::module &m)
{
  PySharedPtrClass< smtk::session::rgg::ReadRXFFile, smtk::operation::XMLOperation > instance(m, "ReadRXFFile");
  instance
    .def(py::init<::smtk::session::rgg::ReadRXFFile const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::session::rgg::ReadRXFFile & (smtk::session::rgg::ReadRXFFile::*)(::smtk::session::rgg::ReadRXFFile const &)) &smtk::session::rgg::ReadRXFFile::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::ReadRXFFile> (*)()) &smtk::session::rgg::ReadRXFFile::create)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::ReadRXFFile> (*)(::std::shared_ptr<smtk::session::rgg::ReadRXFFile> &)) &smtk::session::rgg::ReadRXFFile::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::rgg::ReadRXFFile> (smtk::session::rgg::ReadRXFFile::*)()) &smtk::session::rgg::ReadRXFFile::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::rgg::ReadRXFFile> (smtk::session::rgg::ReadRXFFile::*)() const) &smtk::session::rgg::ReadRXFFile::shared_from_this)
    ;
  return instance;
}

#endif
