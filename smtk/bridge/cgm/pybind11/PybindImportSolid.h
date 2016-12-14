//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_ImportSolid_h
#define pybind_smtk_bridge_cgm_ImportSolid_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/ImportSolid.h"

namespace py = pybind11;

py::class_< smtk::bridge::cgm::ImportSolid > pybind11_init_smtk_bridge_cgm_ImportSolid(py::module &m)
{
  py::class_< smtk::bridge::cgm::ImportSolid > instance(m, "ImportSolid");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::ImportSolid const &>())
    .def("deepcopy", (smtk::bridge::cgm::ImportSolid & (smtk::bridge::cgm::ImportSolid::*)(::smtk::bridge::cgm::ImportSolid const &)) &smtk::bridge::cgm::ImportSolid::operator=)
    .def_static("fromFilenameIntoManager", &smtk::bridge::cgm::ImportSolid::fromFilenameIntoManager, py::arg("filename"), py::arg("filetype"), py::arg("manager"))
    ;
  return instance;
}

#endif
