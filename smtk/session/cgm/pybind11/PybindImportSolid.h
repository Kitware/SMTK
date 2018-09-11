//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_ImportSolid_h
#define pybind_smtk_session_cgm_ImportSolid_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/ImportSolid.h"

namespace py = pybind11;

py::class_< smtk::session::cgm::ImportSolid > pybind11_init_smtk_session_cgm_ImportSolid(py::module &m)
{
  py::class_< smtk::session::cgm::ImportSolid > instance(m, "ImportSolid");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::ImportSolid const &>())
    .def("deepcopy", (smtk::session::cgm::ImportSolid & (smtk::session::cgm::ImportSolid::*)(::smtk::session::cgm::ImportSolid const &)) &smtk::session::cgm::ImportSolid::operator=)
    .def_static("fromFilenameIntoManager", &smtk::session::cgm::ImportSolid::fromFilenameIntoManager, py::arg("filename"), py::arg("filetype"), py::arg("manager"))
    ;
  return instance;
}

#endif
