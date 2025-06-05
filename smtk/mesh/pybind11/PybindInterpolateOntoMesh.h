//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_operators_InterpolateOntoMesh_h
#define pybind_smtk_mesh_operators_InterpolateOntoMesh_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/operators/InterpolateOntoMesh.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::mesh::InterpolateOntoMesh, smtk::operation::XMLOperation > pybind11_init_smtk_mesh_InterpolateOntoMesh(py::module &m)
{
  PySharedPtrClass< smtk::mesh::InterpolateOntoMesh, smtk::operation::XMLOperation > instance(m, "InterpolateOntoMesh");
  instance
    .def(py::init<>())
    .def("ableToOperate", &smtk::mesh::InterpolateOntoMesh::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::mesh::InterpolateOntoMesh> (*)()) &smtk::mesh::InterpolateOntoMesh::create)
    .def_static("create", (std::shared_ptr<smtk::mesh::InterpolateOntoMesh> (*)(::std::shared_ptr<smtk::mesh::InterpolateOntoMesh> &)) &smtk::mesh::InterpolateOntoMesh::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::mesh::InterpolateOntoMesh> (smtk::mesh::InterpolateOntoMesh::*)() const) &smtk::mesh::InterpolateOntoMesh::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::mesh::InterpolateOntoMesh> (smtk::mesh::InterpolateOntoMesh::*)()) &smtk::mesh::InterpolateOntoMesh::shared_from_this)
    ;
  return instance;
}

#endif
