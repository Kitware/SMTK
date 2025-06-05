//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_operators_Write_h
#define pybind_smtk_mesh_operators_Write_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/operators/Write.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::mesh::Write, smtk::operation::XMLOperation > pybind11_init_smtk_mesh_Write(py::module &m)
{
  PySharedPtrClass< smtk::mesh::Write, smtk::operation::XMLOperation > instance(m, "Write");
  instance
    .def(py::init<>())
    .def("ableToOperate", &smtk::mesh::Write::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::mesh::Write> (*)()) &smtk::mesh::Write::create)
    .def_static("create", (std::shared_ptr<smtk::mesh::Write> (*)(::std::shared_ptr<smtk::mesh::Write> &)) &smtk::mesh::Write::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::mesh::Write> (smtk::mesh::Write::*)() const) &smtk::mesh::Write::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::mesh::Write> (smtk::mesh::Write::*)()) &smtk::mesh::Write::shared_from_this)
    ;
  return instance;
}

#endif
