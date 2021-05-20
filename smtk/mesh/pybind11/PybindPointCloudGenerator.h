//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_PointCloudGenerator_h
#define pybind_smtk_mesh_PointCloudGenerator_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/interpolation/PointCloudGenerator.h"
#include "smtk/model/AuxiliaryGeometry.h"

namespace py = pybind11;

inline py::class_<smtk::mesh::PointCloudGenerator> pybind11_init_smtk_mesh_PointCloudGenerator(py::module &m)
{
  py::class_<smtk::mesh::PointCloudGenerator> instance(m, "PointCloudGenerator");
  instance
    .def(py::init<>())
    .def(py::init<smtk::mesh::PointCloudGenerator const &>())
    .def("__call__", (smtk::mesh::PointCloud (smtk::mesh::PointCloudGenerator::*)(smtk::model::AuxiliaryGeometry const &)) &smtk::mesh::PointCloudGenerator::operator())
    .def("__call__", (smtk::mesh::PointCloud (smtk::mesh::PointCloudGenerator::*)(::std::string const &)) &smtk::mesh::PointCloudGenerator::operator())
    .def("deepcopy", (smtk::mesh::PointCloudGenerator & (smtk::mesh::PointCloudGenerator::*)(smtk::mesh::PointCloudGenerator const &)) &smtk::mesh::PointCloudGenerator::operator=)
    .def("valid", (bool (smtk::mesh::PointCloudGenerator::*)(::std::string const&) const) &smtk::mesh::PointCloudGenerator::valid, py::arg("file"))
    .def("valid", (bool (smtk::mesh::PointCloudGenerator::*)(smtk::model::AuxiliaryGeometry const&) const) &smtk::mesh::PointCloudGenerator::valid, py::arg("auxGeom"))
    ;
  return instance;
}

#endif
