//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_mesh_operators_EulerCharacteristicRatio_h
#define pybind_smtk_session_mesh_operators_EulerCharacteristicRatio_h

#include <pybind11/pybind11.h>

#include "smtk/session/mesh/operators/EulerCharacteristicRatio.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::mesh::EulerCharacteristicRatio, smtk::operation::XMLOperation > pybind11_init_smtk_session_mesh_EulerCharacteristicRatio(py::module &m)
{
  PySharedPtrClass< smtk::session::mesh::EulerCharacteristicRatio, smtk::operation::XMLOperation > instance(m, "EulerCharacteristicRatio");
  instance
    .def(py::init<>())
    .def_static("create", (std::shared_ptr<smtk::session::mesh::EulerCharacteristicRatio> (*)()) &smtk::session::mesh::EulerCharacteristicRatio::create)
    .def_static("create", (std::shared_ptr<smtk::session::mesh::EulerCharacteristicRatio> (*)(::std::shared_ptr<smtk::session::mesh::EulerCharacteristicRatio> &)) &smtk::session::mesh::EulerCharacteristicRatio::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::mesh::EulerCharacteristicRatio> (smtk::session::mesh::EulerCharacteristicRatio::*)()) &smtk::session::mesh::EulerCharacteristicRatio::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::mesh::EulerCharacteristicRatio> (smtk::session::mesh::EulerCharacteristicRatio::*)() const) &smtk::session::mesh::EulerCharacteristicRatio::shared_from_this)
    ;
  return instance;
}

#endif
