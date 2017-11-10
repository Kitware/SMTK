//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_StructuredGridGenerator_h
#define pybind_smtk_mesh_StructuredGridGenerator_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/interpolation/StructuredGridGenerator.h"
#include "smtk/model/AuxiliaryGeometry.h"

namespace py = pybind11;

py::class_<smtk::mesh::StructuredGridGenerator> pybind11_init_smtk_mesh_StructuredGridGenerator(py::module &m)
{
  py::class_<smtk::mesh::StructuredGridGenerator> instance(m, "StructuredGridGenerator");
  instance
    .def(py::init<>())
    .def(py::init<smtk::mesh::StructuredGridGenerator const &>())
    .def("__call__", (smtk::mesh::StructuredGrid (smtk::mesh::StructuredGridGenerator::*)(smtk::model::AuxiliaryGeometry const &)) &smtk::mesh::StructuredGridGenerator::operator())
    .def("__call__", (smtk::mesh::StructuredGrid (smtk::mesh::StructuredGridGenerator::*)(::std::string const &)) &smtk::mesh::StructuredGridGenerator::operator())
    .def("deepcopy", (smtk::mesh::StructuredGridGenerator & (smtk::mesh::StructuredGridGenerator::*)(smtk::mesh::StructuredGridGenerator const &)) &smtk::mesh::StructuredGridGenerator::operator=)
    .def("valid", (bool (smtk::mesh::StructuredGridGenerator::*)(::std::string const&) const) &smtk::mesh::StructuredGridGenerator::valid, py::arg("file"))
    .def("valid", (bool (smtk::mesh::StructuredGridGenerator::*)(smtk::model::AuxiliaryGeometry const&) const) &smtk::mesh::StructuredGridGenerator::valid, py::arg("auxGeom"))
    ;
  return instance;
}

#endif
