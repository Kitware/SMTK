//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_rgg_operators_AddMaterial_h
#define pybind_smtk_session_rgg_operators_AddMaterial_h

#include <pybind11/pybind11.h>

#include "smtk/session/rgg/operators/AddMaterial.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::rgg::AddMaterial, smtk::operation::XMLOperation > pybind11_init_smtk_session_rgg_AddMaterial(py::module &m)
{
  PySharedPtrClass< smtk::session::rgg::AddMaterial, smtk::operation::XMLOperation > instance(m, "AddMaterial");
  instance
    .def(py::init<::smtk::session::rgg::AddMaterial const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::session::rgg::AddMaterial & (smtk::session::rgg::AddMaterial::*)(::smtk::session::rgg::AddMaterial const &)) &smtk::session::rgg::AddMaterial::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::AddMaterial> (*)()) &smtk::session::rgg::AddMaterial::create)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::AddMaterial> (*)(::std::shared_ptr<smtk::session::rgg::AddMaterial> &)) &smtk::session::rgg::AddMaterial::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::rgg::AddMaterial> (smtk::session::rgg::AddMaterial::*)()) &smtk::session::rgg::AddMaterial::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::rgg::AddMaterial> (smtk::session::rgg::AddMaterial::*)() const) &smtk::session::rgg::AddMaterial::shared_from_this)
    ;
  return instance;
}

#endif
