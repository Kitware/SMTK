//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_operators_Associate_h
#define pybind_smtk_attribute_operators_Associate_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/operators/Associate.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::Associate, smtk::operation::XMLOperation > pybind11_init_smtk_attribute_Associate(py::module &m)
{
  PySharedPtrClass< smtk::attribute::Associate, smtk::operation::XMLOperation > instance(m, "Associate");
  instance
    .def(py::init<>())
    .def_static("create", (std::shared_ptr<smtk::attribute::Associate> (*)()) &smtk::attribute::Associate::create)
    .def_static("create", (std::shared_ptr<smtk::attribute::Associate> (*)(::std::shared_ptr<smtk::attribute::Associate> &)) &smtk::attribute::Associate::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::attribute::Associate> (smtk::attribute::Associate::*)() const) &smtk::attribute::Associate::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::attribute::Associate> (smtk::attribute::Associate::*)()) &smtk::attribute::Associate::shared_from_this)
    ;
  return instance;
}

#endif
