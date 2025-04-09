//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_operators_Dissociate_h
#define pybind_smtk_attribute_operators_Dissociate_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/operators/Dissociate.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::Dissociate, smtk::operation::XMLOperation > pybind11_init_smtk_attribute_Dissociate(py::module &m)
{
  PySharedPtrClass< smtk::attribute::Dissociate, smtk::operation::XMLOperation > instance(m, "Dissociate");
  instance
    .def(py::init<>())
    .def_static("create", (std::shared_ptr<smtk::attribute::Dissociate> (*)()) &smtk::attribute::Dissociate::create)
    .def_static("create", (std::shared_ptr<smtk::attribute::Dissociate> (*)(::std::shared_ptr<smtk::attribute::Dissociate> &)) &smtk::attribute::Dissociate::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::attribute::Dissociate> (smtk::attribute::Dissociate::*)() const) &smtk::attribute::Dissociate::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::attribute::Dissociate> (smtk::attribute::Dissociate::*)()) &smtk::attribute::Dissociate::shared_from_this)
    ;
  return instance;
}

#endif
