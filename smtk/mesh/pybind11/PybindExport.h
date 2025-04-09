//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_operators_Export_h
#define pybind_smtk_mesh_operators_Export_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/operators/Export.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::mesh::Export, smtk::operation::XMLOperation > pybind11_init_smtk_mesh_Export(py::module &m)
{
  PySharedPtrClass< smtk::mesh::Export, smtk::operation::XMLOperation > instance(m, "Export");
  instance
    .def(py::init<>())
    .def("ableToOperate", &smtk::mesh::Export::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::mesh::Export> (*)()) &smtk::mesh::Export::create)
    .def_static("create", (std::shared_ptr<smtk::mesh::Export> (*)(::std::shared_ptr<smtk::mesh::Export> &)) &smtk::mesh::Export::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::mesh::Export> (smtk::mesh::Export::*)() const) &smtk::mesh::Export::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::mesh::Export> (smtk::mesh::Export::*)()) &smtk::mesh::Export::shared_from_this)
    ;
  return instance;
}

#endif
