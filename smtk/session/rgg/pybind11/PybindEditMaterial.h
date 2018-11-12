//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_rgg_operators_EditMaterial_h
#define pybind_smtk_session_rgg_operators_EditMaterial_h

#include <pybind11/pybind11.h>

#include "smtk/session/rgg/operators/EditMaterial.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::rgg::EditMaterial, smtk::operation::XMLOperation > pybind11_init_smtk_session_rgg_EditMaterial(py::module &m)
{
  PySharedPtrClass< smtk::session::rgg::EditMaterial, smtk::operation::XMLOperation > instance(m, "EditMaterial");
  instance
    .def(py::init<::smtk::session::rgg::EditMaterial const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::session::rgg::EditMaterial & (smtk::session::rgg::EditMaterial::*)(::smtk::session::rgg::EditMaterial const &)) &smtk::session::rgg::EditMaterial::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::EditMaterial> (*)()) &smtk::session::rgg::EditMaterial::create)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::EditMaterial> (*)(::std::shared_ptr<smtk::session::rgg::EditMaterial> &)) &smtk::session::rgg::EditMaterial::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::rgg::EditMaterial> (smtk::session::rgg::EditMaterial::*)()) &smtk::session::rgg::EditMaterial::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::rgg::EditMaterial> (smtk::session::rgg::EditMaterial::*)() const) &smtk::session::rgg::EditMaterial::shared_from_this)
    ;
  return instance;
}

#endif
