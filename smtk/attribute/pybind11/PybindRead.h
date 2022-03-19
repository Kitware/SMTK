//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_operators_Read_h
#define pybind_smtk_attribute_operators_Read_h

#include <pybind11/pybind11.h>

#include "smtk/common/Managers.h"
#include "smtk/attribute/operators/Read.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::Read, smtk::operation::XMLOperation > pybind11_init_smtk_attribute_Read(py::module &m)
{
  PySharedPtrClass< smtk::attribute::Read, smtk::operation::XMLOperation > instance(m, "Read");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::attribute::Read const &>())
    .def("deepcopy", (smtk::attribute::Read & (smtk::attribute::Read::*)(::smtk::attribute::Read const &)) &smtk::attribute::Read::operator=)
    .def_static("create", (std::shared_ptr<smtk::attribute::Read> (*)()) &smtk::attribute::Read::create)
    .def_static("create", (std::shared_ptr<smtk::attribute::Read> (*)(::std::shared_ptr<smtk::attribute::Read> &)) &smtk::attribute::Read::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::attribute::Read> (smtk::attribute::Read::*)() const) &smtk::attribute::Read::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::attribute::Read> (smtk::attribute::Read::*)()) &smtk::attribute::Read::shared_from_this)
    ;

  m.def("read", (smtk::resource::ResourcePtr (*)(::std::string const &, const std::shared_ptr<smtk::common::Managers>&)) &smtk::attribute::read, "", py::arg("filePath"), py::arg("managers") = nullptr);


  return instance;
}

#endif
