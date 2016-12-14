//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_ExportSolid_h
#define pybind_smtk_bridge_cgm_ExportSolid_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/ExportSolid.h"

namespace py = pybind11;

py::class_< smtk::bridge::cgm::ExportSolid > pybind11_init_smtk_bridge_cgm_ExportSolid(py::module &m)
{
  py::class_< smtk::bridge::cgm::ExportSolid > instance(m, "ExportSolid");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::ExportSolid const &>())
    .def("deepcopy", (smtk::bridge::cgm::ExportSolid & (smtk::bridge::cgm::ExportSolid::*)(::smtk::bridge::cgm::ExportSolid const &)) &smtk::bridge::cgm::ExportSolid::operator=)
    .def_static("entitiesToFileOfNameAndType", &smtk::bridge::cgm::ExportSolid::entitiesToFileOfNameAndType, py::arg("entities"), py::arg("filename"), py::arg("filetype"))
    ;
  return instance;
}

#endif
