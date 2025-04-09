//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_operators_ElevateMesh_h
#define pybind_smtk_mesh_operators_ElevateMesh_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/operators/ElevateMesh.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::mesh::ElevateMesh, smtk::operation::XMLOperation > pybind11_init_smtk_mesh_ElevateMesh(py::module &m)
{
  PySharedPtrClass< smtk::mesh::ElevateMesh, smtk::operation::XMLOperation > instance(m, "ElevateMesh");
  instance
    .def(py::init<>())
    .def("ableToOperate", &smtk::mesh::ElevateMesh::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::mesh::ElevateMesh> (*)()) &smtk::mesh::ElevateMesh::create)
    .def_static("create", (std::shared_ptr<smtk::mesh::ElevateMesh> (*)(::std::shared_ptr<smtk::mesh::ElevateMesh> &)) &smtk::mesh::ElevateMesh::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::mesh::ElevateMesh> (smtk::mesh::ElevateMesh::*)() const) &smtk::mesh::ElevateMesh::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::mesh::ElevateMesh> (smtk::mesh::ElevateMesh::*)()) &smtk::mesh::ElevateMesh::shared_from_this)
    ;
  return instance;
}

#endif
