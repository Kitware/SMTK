//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_operators_Read_h
#define pybind_smtk_mesh_operators_Read_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/operators/Read.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::mesh::Read, smtk::operation::XMLOperation > pybind11_init_smtk_mesh_Read(py::module &m)
{
  PySharedPtrClass< smtk::mesh::Read, smtk::operation::XMLOperation > instance(m, "Read");
  instance
    .def(py::init<>())
    .def("ableToOperate", &smtk::mesh::Read::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::mesh::Read> (*)()) &smtk::mesh::Read::create)
    .def_static("create", (std::shared_ptr<smtk::mesh::Read> (*)(::std::shared_ptr<smtk::mesh::Read> &)) &smtk::mesh::Read::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::mesh::Read> (smtk::mesh::Read::*)() const) &smtk::mesh::Read::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::mesh::Read> (smtk::mesh::Read::*)()) &smtk::mesh::Read::shared_from_this)
    ;
  return instance;
}

#endif
