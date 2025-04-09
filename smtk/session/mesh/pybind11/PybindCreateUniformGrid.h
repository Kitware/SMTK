//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_mesh_operators_CreateUniformGrid_h
#define pybind_smtk_session_mesh_operators_CreateUniformGrid_h

#include <pybind11/pybind11.h>

#include "smtk/session/mesh/operators/CreateUniformGrid.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::mesh::CreateUniformGrid, smtk::operation::XMLOperation > pybind11_init_smtk_session_mesh_CreateUniformGrid(py::module &m)
{
  PySharedPtrClass< smtk::session::mesh::CreateUniformGrid, smtk::operation::XMLOperation > instance(m, "CreateUniformGrid");
  instance
    .def(py::init<>())
    .def_static("create", (std::shared_ptr<smtk::session::mesh::CreateUniformGrid> (*)()) &smtk::session::mesh::CreateUniformGrid::create)
    .def_static("create", (std::shared_ptr<smtk::session::mesh::CreateUniformGrid> (*)(::std::shared_ptr<smtk::session::mesh::CreateUniformGrid> &)) &smtk::session::mesh::CreateUniformGrid::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::mesh::CreateUniformGrid> (smtk::session::mesh::CreateUniformGrid::*)()) &smtk::session::mesh::CreateUniformGrid::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::mesh::CreateUniformGrid> (smtk::session::mesh::CreateUniformGrid::*)() const) &smtk::session::mesh::CreateUniformGrid::shared_from_this)
    ;
  return instance;
}

#endif
