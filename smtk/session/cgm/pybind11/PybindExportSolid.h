//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_ExportSolid_h
#define pybind_smtk_session_cgm_ExportSolid_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/ExportSolid.h"

namespace py = pybind11;

py::class_< smtk::session::cgm::ExportSolid > pybind11_init_smtk_session_cgm_ExportSolid(py::module &m)
{
  py::class_< smtk::session::cgm::ExportSolid > instance(m, "ExportSolid");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::ExportSolid const &>())
    .def("deepcopy", (smtk::session::cgm::ExportSolid & (smtk::session::cgm::ExportSolid::*)(::smtk::session::cgm::ExportSolid const &)) &smtk::session::cgm::ExportSolid::operator=)
    .def_static("entitiesToFileOfNameAndType", &smtk::session::cgm::ExportSolid::entitiesToFileOfNameAndType, py::arg("entities"), py::arg("filename"), py::arg("filetype"))
    ;
  return instance;
}

#endif
