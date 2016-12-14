//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_ModelToMesh_h
#define pybind_smtk_io_ModelToMesh_h

#include <pybind11/pybind11.h>

#include "smtk/io/ModelToMesh.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::ModelToMesh > pybind11_init_smtk_io_ModelToMesh(py::module &m)
{
  PySharedPtrClass< smtk::io::ModelToMesh > instance(m, "ModelToMesh");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::io::ModelToMesh const &>())
    .def("__call__", (smtk::mesh::CollectionPtr (smtk::io::ModelToMesh::*)(::smtk::mesh::ManagerPtr const &, ::smtk::model::ManagerPtr const &) const) &smtk::io::ModelToMesh::operator())
    .def("__call__", (smtk::mesh::CollectionPtr (smtk::io::ModelToMesh::*)(::smtk::model::Model const &) const) &smtk::io::ModelToMesh::operator())
    .def("deepcopy", (smtk::io::ModelToMesh & (smtk::io::ModelToMesh::*)(::smtk::io::ModelToMesh const &)) &smtk::io::ModelToMesh::operator=)
    .def("isMergingEnabled", &smtk::io::ModelToMesh::isMergingEnabled)
    .def("setIsMerging", &smtk::io::ModelToMesh::setIsMerging, py::arg("m"))
    .def("getMergeTolerance", &smtk::io::ModelToMesh::getMergeTolerance)
    .def("setMergeTolerance", &smtk::io::ModelToMesh::setMergeTolerance, py::arg("tol"))
    ;
  return instance;
}

#endif
