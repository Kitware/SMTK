//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_mesh_operators_Import_h
#define pybind_smtk_session_mesh_operators_Import_h

#include <pybind11/pybind11.h>

#include "smtk/session/mesh/operators/Import.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::mesh::Import, smtk::operation::XMLOperation > pybind11_init_smtk_session_mesh_Import(py::module &m)
{
  PySharedPtrClass< smtk::session::mesh::Import, smtk::operation::XMLOperation > instance(m, "Import");
  instance
    .def(py::init<>())
    .def_static("create", (std::shared_ptr<smtk::session::mesh::Import> (*)()) &smtk::session::mesh::Import::create)
    .def_static("create", (std::shared_ptr<smtk::session::mesh::Import> (*)(::std::shared_ptr<smtk::session::mesh::Import> &)) &smtk::session::mesh::Import::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::mesh::Import> (smtk::session::mesh::Import::*)()) &smtk::session::mesh::Import::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::mesh::Import> (smtk::session::mesh::Import::*)() const) &smtk::session::mesh::Import::shared_from_this)
    ;
  return instance;
}

#endif
