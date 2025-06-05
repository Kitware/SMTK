//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_operators_Write_h
#define pybind_smtk_attribute_operators_Write_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/operators/Write.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::Write, smtk::operation::XMLOperation > pybind11_init_smtk_attribute_Write(py::module &m)
{
  PySharedPtrClass< smtk::attribute::Write, smtk::operation::XMLOperation > instance(m, "Write");
  instance
    .def(py::init<>())
    .def_static("create", (std::shared_ptr<smtk::attribute::Write> (*)()) &smtk::attribute::Write::create)
    .def_static("create", (std::shared_ptr<smtk::attribute::Write> (*)(::std::shared_ptr<smtk::attribute::Write> &)) &smtk::attribute::Write::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::attribute::Write> (smtk::attribute::Write::*)() const) &smtk::attribute::Write::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::attribute::Write> (smtk::attribute::Write::*)()) &smtk::attribute::Write::shared_from_this)
    ;

  m.def("write", (bool (*)(const smtk::resource::ResourcePtr&, const std::shared_ptr<smtk::common::Managers>&)) &smtk::attribute::write, "", py::arg("resource"), py::arg("managers") = nullptr);

  return instance;
}

#endif
